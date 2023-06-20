#include <iostream>
#include <filesystem>

#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>
#include <Yuki/EventSystem/ApplicationEvents.hpp>
#include <Yuki/Rendering/RHI/GraphicsPipelineBuilder.hpp>
#include <Yuki/Rendering/RHI/ShaderCompiler.hpp>
#include <Yuki/Rendering/RHI/RenderTarget.hpp>
#include <Yuki/Rendering/RHI/Queue.hpp>
#include <Yuki/Rendering/RHI/Fence.hpp>
#include <Yuki/Rendering/SceneRenderer.hpp>
#include <Yuki/IO/MeshLoader.hpp>

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

		m_Renderer = new Yuki::SceneRenderer(GetRenderContext(), m_Windows[0]->GetViewport());

		//m_ColorAttachment = GetRenderContext()->CreateImage2D(1920, 1080, Yuki::ImageFormat::BGRA8UNorm);
		//m_DepthAttachment = GetRenderContext()->CreateImage2D(1920, 1080, Yuki::ImageFormat::Depth24UNorm);

		m_Mesh = Yuki::Mesh::FromMeshData(GetRenderContext(), Yuki::MeshLoader::LoadGLTFMesh("Resources/Meshes/NewSponza_Main_glTF_002.gltf"));
	}

	void OnRunLoop() override
	{
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

		/*Yuki::RenderTarget renderTarget =
		{
			.ColorAttachments = { m_ColorAttachment->GetDefaultImageView() },
			.DepthAttachment = m_DepthAttachment->GetDefaultImageView()
		};*/

		m_Renderer->BeginDraw();
		m_Renderer->DrawMesh(m_Mesh);
		m_Renderer->EndDraw();

		GetRenderContext()->GetGraphicsQueue()->SubmitCommandBuffers({ m_Renderer->GetCurrentCommandBuffer() }, { m_Fence }, { m_Fence });

		// Present all swapchain images
		GetRenderContext()->GetGraphicsQueue()->Present(viewports, { m_Fence });
	}

	void OnDestroy() override
	{
		GetRenderContext()->DestroyFence(m_Fence);
	}

private:
	std::vector<Yuki::GenericWindow*> m_Windows;
	Yuki::Fence* m_Fence = nullptr;

	Yuki::SceneRenderer* m_Renderer = nullptr;

	Yuki::Image2D* m_ColorAttachment = nullptr;
	Yuki::Image2D* m_DepthAttachment = nullptr;

	Yuki::Mesh m_Mesh;
};

YUKI_DECLARE_APPLICATION(TestApplication)
