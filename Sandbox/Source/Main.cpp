#include <iostream>
#include <filesystem>


#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>
#include <Yuki/Math/Math.hpp>
#include <Yuki/EventSystem/ApplicationEvents.hpp>
#include <Yuki/Rendering/RHI/GraphicsPipelineBuilder.hpp>
#include <Yuki/Rendering/RHI/ShaderCompiler.hpp>
#include <Yuki/Rendering/RHI/RenderTarget.hpp>
#include <Yuki/Rendering/RHI/Queue.hpp>
#include <Yuki/Rendering/RHI/Fence.hpp>
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

		m_Renderer = new Yuki::SceneRenderer(GetRenderContext());

		m_Mesh = Yuki::MeshLoader::LoadGLTFMesh(GetRenderContext(), "Resources/Meshes/deccer-cubes/SM_Deccer_Cubes_Textured_Complex.gltf");
		//m_Mesh = Yuki::MeshLoader::LoadGLTFMesh(GetRenderContext(), "Resources/Meshes/NewSponza_Main_glTF_002.gltf");

		m_Camera = Yuki::Unique<FreeCamera>::Create(m_Windows[0]);
	}

	void OnRunLoop() override
	{
		m_Camera->Update(0.0f);

		// Collect Viewports
		std::vector<Yuki::Viewport*> viewports;
		viewports.reserve(m_Windows.size());
		for (auto* window : m_Windows)
		{
			if (window == nullptr || window->GetViewport() == nullptr)
				continue;

			viewports.emplace_back(window->GetViewport());
		}

		m_Fence->Wait();

		// Acquire Images for all Viewports
		GetRenderContext()->GetGraphicsQueue()->AcquireImages(viewports, { m_Fence });

		m_Renderer->BeginFrame();

		if (!viewports.empty() && viewports[0])
		{
			m_Renderer->SetTargetViewport(viewports[0]);
			m_Renderer->BeginDraw(m_Camera->GetViewMatrix());
			m_Renderer->DrawMesh(m_Mesh);
			m_Renderer->EndDraw();
		}

		m_Renderer->EndFrame();
		GetRenderContext()->GetGraphicsQueue()->SubmitCommandBuffers({ m_Renderer->GetCurrentCommandBuffer() }, { m_Fence }, {});

		// Present all swapchain images
		GetRenderContext()->GetGraphicsQueue()->Present(viewports, { m_Fence });
	}

	void OnDestroy() override
	{
		m_Mesh.Meshes.clear();
		m_Mesh.Instances.clear();
		m_Mesh.LoadedImages.clear();
		m_Mesh.Textures.clear();
		m_Mesh.Materials.clear();

		m_Fence.Release();

		delete m_Renderer;
	}

private:
	std::vector<Yuki::GenericWindow*> m_Windows;
	Yuki::Unique<Yuki::Fence> m_Fence = nullptr;

	Yuki::Unique<FreeCamera> m_Camera = nullptr;

	Yuki::SceneRenderer* m_Renderer = nullptr;

	Yuki::LoadedMesh m_Mesh;
};

YUKI_DECLARE_APPLICATION(TestApplication)
