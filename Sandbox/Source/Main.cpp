#include <iostream>
#include <filesystem>
#include <array>
#include <span>

#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>
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

		m_Mesh = Yuki::MeshLoader::LoadGLTFMesh(GetRenderContext(), "Resources/Meshes/NewSponza_Main_glTF_002.gltf");
		m_Mesh2 = Yuki::MeshLoader::LoadGLTFMesh(GetRenderContext(), "Resources/Meshes/deccer-cubes/SM_Deccer_Cubes_Textured_Complex.gltf");

		/*
		TODO(Peter):
			- Implement Proxy Objects over the direct RenderContext interface
			- Multiple frames in flight
			- Make sure multiple windows / swapchains still works
		*/

		m_Camera = Yuki::Unique<FreeCamera>::Create(m_Windows[0]);
	}

	void OnRunLoop() override
	{
		m_Camera->Update(0.0f);

		// Collect Swapchains
		auto swapchains = GetRenderContext()->GetSwapchains();

		if (swapchains.empty())
			return;

		static bool s_Sponza = true;
		if (m_Windows[0]->IsKeyPressed(Yuki::KeyCode::Num1))
			s_Sponza = true;
		else if (m_Windows[0]->IsKeyPressed(Yuki::KeyCode::Num2))
			s_Sponza = false;

		GetRenderContext()->FenceWait(m_Fence);

		// Acquire Images for all Viewports
		GetRenderContext()->QueueAcquireImages(swapchains, { m_Fence });

		m_Renderer->BeginFrame(Yuki::Math::Mat4::PerspectiveInfReversedZ(70.0f, 1920.0f / 1080.0f, 0.05f) * m_Camera->GetViewMatrix());
		m_Renderer->Submit(s_Sponza ? m_Mesh : m_Mesh2);
		m_Renderer->EndFrame(m_Fence);

		// Present all swapchain images
		GetRenderContext()->QueuePresent(swapchains, { m_Fence });
	}

	void OnDestroy() override
	{
		delete m_Renderer;
	}

private:
	std::vector<Yuki::GenericWindow*> m_Windows;
	Yuki::Fence m_Fence{};

	Yuki::Unique<FreeCamera> m_Camera = nullptr;

	Yuki::SceneRenderer* m_Renderer = nullptr;

	Yuki::LoadedMesh m_Mesh;
	Yuki::LoadedMesh m_Mesh2;
};

YUKI_DECLARE_APPLICATION(TestApplication)
