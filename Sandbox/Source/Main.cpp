#include <iostream>
#include <filesystem>
#include <array>
#include <span>

#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>
#include <Yuki/Core/Timer.hpp>
#include <Yuki/Math/Math.hpp>
#include <Yuki/Math/Mat4.hpp>
#include <Yuki/EventSystem/ApplicationEvents.hpp>
#include <Yuki/Rendering/PipelineBuilder.hpp>
#include <Yuki/Rendering/DescriptorSetBuilder.hpp>
#include <Yuki/Rendering/SceneRenderer.hpp>
#include <Yuki/IO/MeshLoader.hpp>

#include <Yuki/ImGui/ImGuiWindowContext.hpp>
#include <Yuki/ImGui/ImGuiRenderContext.hpp>
#include <imgui/imgui.h>

#include "FreeCamera.hpp"

class TestApplication : public Yuki::Application
{
public:
	TestApplication()
	    : Yuki::Application("Test Application")
	{
	}

private:
	void CreateWindow(Yuki::WindowAttributes InWindowAttributes)
	{
		auto* window = NewWindow(std::move(InWindowAttributes));
		window->Show();
		m_Windows.emplace_back(window);
	}

	void OnInitialize() override
	{
		CreateWindow({
			.Title = "My Main Window",
			.Width = 1920,
			.Height = 1080
		});

		m_GraphicsQueue = { GetRenderContext()->GetGraphicsQueue(), GetRenderContext() };

		m_Fence = Yuki::Fence(GetRenderContext());

		m_Renderer = new Yuki::SceneRenderer(GetRenderContext(), m_Windows[0]->GetSwapchain());

		m_MeshLoader = Yuki::Unique<Yuki::MeshLoader>::Create(GetRenderContext(), [this](Yuki::Mesh InMesh)
		{
			std::scoped_lock lock(m_MeshesMutex);
			m_Meshes.emplace_back(std::move(InMesh));
			m_MeshDataUploadQueue.emplace_back(m_Meshes.size() - 1);
		});

		m_MeshLoader->LoadGLTFMesh("Resources/Meshes/deccer-cubes/SM_Deccer_Cubes_Textured_Complex.gltf");
		m_MeshLoader->LoadGLTFMesh("Resources/Meshes/NewSponza_Main_glTF_002.gltf");
		//m_MeshLoader->LoadGLTFMesh("Resources/Meshes/Small_City_LVL/Small_City_LVL.gltf");
		//m_MeshLoader->LoadGLTFMesh("Resources/Meshes/powerplant/powerplant.gltf");

		m_Camera = Yuki::Unique<FreeCamera>::Create(m_Windows[0]);

		InitializeImGui();
	}

	void InitializeImGui()
	{
		ImGui::SetCurrentContext(ImGui::CreateContext());
		auto& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigWindowsMoveFromTitleBarOnly = true;

		ImGui::StyleColorsDark();

		m_ImGuiWindowContext = Yuki::Unique<Yuki::ImGuiWindowContext>::Create(m_Windows[0]);
		m_ImGuiRenderContext = Yuki::ImGuiRenderContext::New(m_Windows[0]->GetSwapchain(), GetRenderContext());

		m_CommandPool = Yuki::CommandPool(GetRenderContext(), m_GraphicsQueue);
		m_ImGuiCommandList = m_CommandPool.CreateCommandList();

		auto& style = ImGui::GetStyle();
		style.FramePadding = ImVec2(6.0f, 6.0f);
	}

	void OnRunLoop(float InDeltaTime) override
	{
		m_Camera->Update(InDeltaTime);

		// Collect Swapchains
		auto swapchains = GetRenderContext()->GetSwapchains();

		if (swapchains.empty())
			return;

		m_Fence.Wait();

		// Acquire Images for all Viewports
		m_GraphicsQueue.AcquireImages(swapchains, { m_Fence });

		{
			std::scoped_lock lock(m_MeshesMutex);

			const auto& windowAttribs = m_Windows[0]->GetAttributes();
			m_Renderer->BeginFrame(Yuki::Math::Mat4::PerspectiveInfReversedZ(70.0f, (float)windowAttribs.Width / windowAttribs.Height, 0.05f) * m_Camera->GetViewMatrix());

			while (!m_MeshDataUploadQueue.empty())
			{
				m_Renderer->RegisterMeshData(m_Meshes[m_MeshDataUploadQueue.back()]);
				m_MeshDataUploadQueue.pop_back();
			}

			for (auto& mesh : m_Meshes)
				m_Renderer->Submit(mesh);

			m_Renderer->EndFrame(m_Fence);
		}

		m_CommandPool.Reset();

		m_ImGuiCommandList.Begin();

		m_ImGuiRenderContext->NewFrame(m_ImGuiCommandList);
		m_ImGuiWindowContext->NewFrame();
		ImGui::NewFrame();

		DrawUI();

		ImGui::Render();
		m_ImGuiRenderContext->EndFrame(m_ImGuiCommandList);
		m_ImGuiCommandList.PrepareSwapchainPresent(m_Windows[0]->GetSwapchain());
		m_ImGuiCommandList.End();

		m_GraphicsQueue.SubmitCommandLists({ m_ImGuiCommandList }, { m_Fence }, { m_Fence });

		// Present all swapchain images
		m_GraphicsQueue.Present(swapchains, { m_Fence });
	}

	void DrawUI()
	{
		if (ImGui::Begin("Settings"))
		{
			ImGui::DragFloat("Camera Speed", &m_Camera->GetMovementSpeed());
		}
		ImGui::End();
	}

	void OnDestroy() override
	{
	}

private:
	std::vector<Yuki::GenericWindow*> m_Windows;
	Yuki::Queue m_GraphicsQueue{};
	Yuki::Fence m_Fence{};

	Yuki::Unique<FreeCamera> m_Camera = nullptr;
	Yuki::Unique<Yuki::MeshLoader> m_MeshLoader = nullptr;

	Yuki::SceneRenderer* m_Renderer = nullptr;

	std::shared_mutex m_MeshesMutex;
	Yuki::DynamicArray<Yuki::Mesh> m_Meshes;
	Yuki::DynamicArray<size_t> m_MeshDataUploadQueue;

	Yuki::Unique<Yuki::ImGuiWindowContext> m_ImGuiWindowContext;
	Yuki::Unique<Yuki::ImGuiRenderContext> m_ImGuiRenderContext;

	Yuki::CommandPool m_CommandPool{};
	Yuki::CommandList m_ImGuiCommandList{};
};

YUKI_DECLARE_APPLICATION(TestApplication)
