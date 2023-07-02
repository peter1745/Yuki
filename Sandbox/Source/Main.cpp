#include "FreeCamera.hpp"

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
#include <Yuki/Rendering/EntityRenderer.hpp>
#include <Yuki/IO/MeshLoader.hpp>
#include <Yuki/Core/ResourceRegistry.hpp>

#include <Yuki/Entities/TransformComponents.hpp>
#include <Yuki/Entities/RenderingComponents.hpp>

#include <Yuki/ImGui/ImGuiWindowContext.hpp>
#include <Yuki/ImGui/ImGuiRenderContext.hpp>

#include <imgui/imgui.h>
#include <flecs/flecs.h>

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

		m_Renderer = new Yuki::EntityRenderer(GetRenderContext(), m_World);

		m_MeshLoader = Yuki::Unique<Yuki::MeshLoader>::Create(GetRenderContext(), [this](Yuki::Mesh InMesh)
		{
			auto handle = m_Renderer->SubmitForUpload(std::move(InMesh));
			std::scoped_lock lock(m_Mutex);
			m_LoadedMeshes.push_back(handle);
		});

		m_MeshLoader->LoadGLTFMesh("Resources/Meshes/deccer-cubes/SM_Deccer_Cubes_Textured_Complex.gltf");
		//m_MeshLoader->LoadGLTFMesh("Resources/Meshes/NewSponza_Main_glTF_002.gltf");
		//m_MeshLoader->LoadGLTFMesh("Resources/Meshes/Small_City_LVL/Small_City_LVL.gltf");
		//m_MeshLoader->LoadGLTFMesh("Resources/Meshes/powerplant/powerplant.gltf");

		m_Camera = Yuki::Unique<FreeCamera>::Create(m_Windows[0]);

		m_CameraEntity = m_World.entity("Camera Entity").add<Yuki::Entities::CameraComponent>();

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

		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
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

		m_Renderer->Reset();

		m_World.progress();

		GetRenderContext()->GetTransferScheduler().Execute();

		if (m_ViewportWidth != m_LastViewportWidth || m_ViewportHeight != m_LastViewportHeight)
		{
			m_Renderer->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
			m_ImGuiRenderContext->RecreateImage(m_Renderer->GetFinalImage());
		}

		m_Renderer->BeginFrame(Yuki::Math::Mat4::PerspectiveInfReversedZ(70.0f, (float)m_ViewportWidth / m_ViewportHeight, 0.05f) * m_Camera->GetViewMatrix());
		m_Renderer->RenderEntities();
		m_Renderer->EndFrame();

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

	void BeginMainDockspace()
	{
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(mainViewport->Pos);
		ImGui::SetNextWindowSize(mainViewport->Size);
		ImGui::SetNextWindowViewport(mainViewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus;

		ImGui::Begin("MainDockspaceWindow", nullptr, flags);
		ImGui::DockSpace(ImGui::GetID("MainDockspace"));

		ImGui::PopStyleVar();
	}

	void EndMainDockspace()
	{
		ImGui::End();
	}

	void DrawUI()
	{
		BeginMainDockspace();

		if (ImGui::Begin("Settings"))
		{
			ImGui::DragFloat("Camera Speed", &m_Camera->GetMovementSpeed());

			std::scoped_lock lock(m_Mutex);
			for (auto handle : m_LoadedMeshes)
			{
				auto label = fmt::format("Mesh {}", static_cast<int32_t>(handle));
				if (ImGui::Button(label.c_str()))
				{
					m_World.entity()
						.set<Yuki::Entities::MeshComponent>({ handle });
				}
			}
		}
		ImGui::End();

		ImVec2 viewportSize{0.0f, 0.0f};
		m_LastViewportWidth = m_ViewportWidth;
		m_LastViewportHeight = m_ViewportHeight;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (ImGui::Begin("Viewport"))
		{
			viewportSize = ImGui::GetContentRegionAvail();
			m_ImGuiRenderContext->DrawImage(m_Renderer->GetFinalImage(), viewportSize);
		}
		ImGui::End();
		ImGui::PopStyleVar();

		if (uint32_t(viewportSize.x) > 0 && uint32_t(viewportSize.y) > 0)
		{
			m_ViewportWidth = uint32_t(viewportSize.x);
			m_ViewportHeight = uint32_t(viewportSize.y);
		}

		EndMainDockspace();
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

	Yuki::EntityRenderer* m_Renderer;

	Yuki::Unique<Yuki::ImGuiWindowContext> m_ImGuiWindowContext;
	Yuki::Unique<Yuki::ImGuiRenderContext> m_ImGuiRenderContext;

	Yuki::CommandPool m_CommandPool{};
	Yuki::CommandList m_ImGuiCommandList{};

	std::shared_mutex m_Mutex;
	Yuki::DynamicArray<Yuki::MeshHandle> m_LoadedMeshes;

	flecs::world m_World;
	flecs::entity m_CameraEntity;

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;
	uint32_t m_LastViewportWidth = 0;
	uint32_t m_LastViewportHeight = 0;
};

YUKI_DECLARE_APPLICATION(TestApplication)
