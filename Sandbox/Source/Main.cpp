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

		m_Renderer = Yuki::Unique<Yuki::SceneRenderer>::Create(GetRenderContext(), m_Windows[0]->GetSwapchain());

		m_MeshLoader = Yuki::Unique<Yuki::MeshLoader>::Create(GetRenderContext(), [this](Yuki::Mesh InMesh)
		{
			std::scoped_lock lock(m_MeshesMutex);
			m_Meshes.emplace_back(std::move(InMesh));
		});

		m_MeshLoader->LoadGLTFMesh("Resources/Meshes/NewSponza_Main_glTF_002.gltf");

		/*
		TODO(Peter):
			- Implement Proxy Objects over the direct RenderContext interface
			- Multiple frames in flight
			- Make sure multiple windows / swapchains still works
		*/

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

		const auto& windowAttribs = m_Windows[0]->GetAttributes();
		m_Renderer->BeginFrame(Yuki::Math::Mat4::PerspectiveInfReversedZ(70.0f, (float)windowAttribs.Width / windowAttribs.Height, 0.05f) * m_Camera->GetViewMatrix());
		std::scoped_lock lock(m_MeshesMutex);
		for (const auto& mesh : m_Meshes)
			m_Renderer->Submit(mesh);
		m_Renderer->EndFrame(m_Fence);

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

	Yuki::Unique<Yuki::SceneRenderer> m_Renderer = nullptr;

	std::shared_mutex m_MeshesMutex;
	Yuki::DynamicArray<Yuki::Mesh> m_Meshes;
};

YUKI_DECLARE_APPLICATION(TestApplication)
