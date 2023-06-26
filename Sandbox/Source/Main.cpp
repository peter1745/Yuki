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

		m_Fence = GetRenderContext()->CreateFence();

		m_Renderer = new Yuki::SceneRenderer(GetRenderContext(), m_Windows[0]->GetSwapchain());

		m_MeshLoader = Yuki::Unique<Yuki::MeshLoader>::Create(GetRenderContext(), [this](Yuki::Mesh InMesh)
		{
			std::scoped_lock lock(m_MeshesMutex);
			m_Meshes.emplace_back(std::move(InMesh));
			m_MeshDataUploadQueue.emplace_back(m_Meshes.size() - 1);
		});

		m_MeshLoader->LoadGLTFMesh("Resources/Meshes/deccer-cubes/SM_Deccer_Cubes_Textured_Complex.gltf");
		m_MeshLoader->LoadGLTFMesh("Resources/Meshes/NewSponza_Main_glTF_002.gltf");
		m_MeshLoader->LoadGLTFMesh("Resources/Meshes/Small_City_LVL/Small_City_LVL.gltf");
		//m_MeshLoader->LoadGLTFMesh("Resources/Meshes/powerplant/powerplant.gltf");

		m_Camera = Yuki::Unique<FreeCamera>::Create(m_Windows[0]);
	}

	void OnRunLoop(float InDeltaTime) override
	{
		m_Camera->Update(InDeltaTime);

		// Collect Swapchains
		auto swapchains = GetRenderContext()->GetSwapchains();

		if (swapchains.empty())
			return;

		GetRenderContext()->FenceWait(m_Fence);

		// Acquire Images for all Viewports
		GetRenderContext()->QueueAcquireImages(GetRenderContext()->GetGraphicsQueue(), swapchains, { m_Fence });

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

		// Present all swapchain images
		GetRenderContext()->QueuePresent(GetRenderContext()->GetGraphicsQueue(), swapchains, { m_Fence });
	}

	void OnDestroy() override
	{
	}

private:
	std::vector<Yuki::GenericWindow*> m_Windows;
	Yuki::Fence m_Fence{};

	Yuki::Unique<FreeCamera> m_Camera = nullptr;
	Yuki::Unique<Yuki::MeshLoader> m_MeshLoader = nullptr;

	Yuki::SceneRenderer* m_Renderer = nullptr;

	std::recursive_mutex m_MeshesMutex;
	Yuki::DynamicArray<Yuki::Mesh> m_Meshes;
	Yuki::DynamicArray<size_t> m_MeshDataUploadQueue;
};

YUKI_DECLARE_APPLICATION(TestApplication)
