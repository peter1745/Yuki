#include <iostream>
#include <filesystem>
#include <array>
#include <span>

#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>
#include <Yuki/Math/Math.hpp>
#include <Yuki/EventSystem/ApplicationEvents.hpp>
#include <Yuki/Rendering/PipelineBuilder.hpp>
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

		m_CommandPool = GetRenderContext()->CreateCommandPool();

		//m_Renderer = new Yuki::SceneRenderer(GetRenderContext());

		//m_Mesh = Yuki::MeshLoader::LoadGLTFMesh(GetRenderContext(), "Resources/Meshes/deccer-cubes/SM_Deccer_Cubes_Textured_Complex.gltf");
		m_Mesh = Yuki::MeshLoader::LoadGLTFMesh(GetRenderContext(), "Resources/Meshes/NewSponza_Main_glTF_002.gltf");

		Yuki::Buffer stagingBuffer = GetRenderContext()->CreateBuffer({
			.Type = Yuki::BufferType::StagingBuffer,
			.Size = 100 * 1024 * 1024,
		});

		{
			m_Mesh.Textures.resize(m_Mesh.LoadedImages.size());
			for (size_t i = 0; i < m_Mesh.LoadedImages.size(); i++)
			{
				auto commandList = GetRenderContext()->CreateCommandList(m_CommandPool);
				GetRenderContext()->CommandListBegin(commandList);

				const auto& imageData = m_Mesh.LoadedImages[i];

				Yuki::Image blittedImage{};
				bool blitted = false;
				Yuki::Image image = GetRenderContext()->CreateImage(imageData.Width, imageData.Height, Yuki::ImageFormat::RGBA8UNorm, Yuki::ImageUsage::Sampled | Yuki::ImageUsage::TransferSource | Yuki::ImageUsage::TransferDestination);
				GetRenderContext()->CommandListTransitionImage(commandList, image, Yuki::ImageLayout::ShaderReadOnly);
				GetRenderContext()->BufferSetData(stagingBuffer, imageData.Data.data(), uint32_t(imageData.Data.size()));
				GetRenderContext()->CommandListCopyToImage(commandList, image, stagingBuffer, 0);

				if (imageData.Width > 2048 && imageData.Height > 2048)
				{
					blittedImage = GetRenderContext()->CreateImage(2048, 2048, Yuki::ImageFormat::RGBA8UNorm, Yuki::ImageUsage::Sampled | Yuki::ImageUsage::TransferDestination);
					GetRenderContext()->CommandListTransitionImage(commandList, blittedImage, Yuki::ImageLayout::ShaderReadOnly);
					GetRenderContext()->CommandListBlitImage(commandList, blittedImage, image);
					blitted = true;
				}

				GetRenderContext()->CommandListEnd(commandList);
				GetRenderContext()->QueueSubmitCommandLists({ commandList }, {}, {});
				GetRenderContext()->DeviceWaitIdle();

				if (blitted)
				{
					GetRenderContext()->Destroy(image);
					image = blittedImage;
				}

				m_Mesh.Textures[i] = image;
			}
		}

		m_Shader = GetRenderContext()->CreateShader("Resources/Shaders/Test.glsl");

		struct FrameTransforms
		{
			Yuki::Math::Mat4 ViewProjection;
			Yuki::Math::Mat4 Transform;
		} m_FrameTransforms;

		/*
		TODO(Peter):
			- Implemented DescriptorSetBuilder (Follow PipelineBuilder implementation)
			- Ensure proper cleanup on shutdown
			- Implement Proxy Objects over the direct RenderContext interface
			- Multiple frames in flight
			- Make sure multiple windows / swapchains still works
		*/

		m_Pipeline = Yuki::PipelineBuilder(GetRenderContext())
			.WithShader(m_Shader)
			.ColorAttachment(Yuki::ImageFormat::BGRA8UNorm)
			.DepthAttachment()
			.AddVertexInput(Yuki::ShaderDataType::Float3)
			.AddVertexInput(Yuki::ShaderDataType::Float3)
			.AddVertexInput(Yuki::ShaderDataType::Float2)
			.AddVertexInput(Yuki::ShaderDataType::UInt)
			//.PushConstant(sizeof(FrameTransforms))
			.Build();

		//.AddDescriptorSetLayout(m_MaterialDescriptorSet->GetLayout())

		struct Vertex
		{
			Yuki::Math::Vec3 Position;
			Yuki::Math::Vec3 Normal{};
			Yuki::Math::Vec2 UV{};
			uint32_t Material{};
		};

		Vertex vertices[] = {
			{ { -0.5f,  0.5f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f } },
			{ {  0.0f, -0.5f, 0.0f } }
		};

		m_VertexBuffer = GetRenderContext()->CreateBuffer({
			.Type = Yuki::BufferType::VertexBuffer,
			.Size = sizeof(Vertex) * 3
		});

		
		GetRenderContext()->BufferSetData(stagingBuffer, vertices, sizeof(Vertex) * 3);

		auto commandList = GetRenderContext()->CreateCommandList(m_CommandPool);
		GetRenderContext()->CommandListBegin(commandList);
		GetRenderContext()->CommandListCopyToBuffer(commandList, m_VertexBuffer, 0, stagingBuffer, 0, 0);
		GetRenderContext()->CommandListEnd(commandList);
		GetRenderContext()->QueueSubmitCommandLists({ commandList}, {}, {});
		GetRenderContext()->DeviceWaitIdle();

		m_Camera = Yuki::Unique<FreeCamera>::Create(m_Windows[0]);
	}

	void OnRunLoop() override
	{
		m_Camera->Update(0.0f);

		// Collect Swapchains
		std::vector<Yuki::Swapchain> swapchains;
		swapchains.reserve(m_Windows.size());
		for (auto* window : m_Windows)
		{
			if (window == nullptr)
				continue;

			swapchains.emplace_back(window->GetSwapchain());
		}

		GetRenderContext()->FenceWait(m_Fence);

		// Acquire Images for all Viewports
		GetRenderContext()->QueueAcquireImages(swapchains, { m_Fence });

		auto commandList = GetRenderContext()->CreateCommandList(m_CommandPool);
		GetRenderContext()->CommandListBegin(commandList);
		GetRenderContext()->CommandListBindPipeline(commandList, m_Pipeline);
		GetRenderContext()->CommandListBindBuffer(commandList, m_VertexBuffer);
		GetRenderContext()->CommandListBeginRendering(commandList, swapchains[0]);
		GetRenderContext()->CommandListDraw(commandList, 3);
		GetRenderContext()->CommandListEndRendering(commandList);
		GetRenderContext()->CommandListEnd(commandList);
		GetRenderContext()->QueueSubmitCommandLists({ commandList}, { m_Fence }, {});

		// Present all swapchain images
		GetRenderContext()->QueuePresent(swapchains, { m_Fence });

#if 0
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
#endif
	}

	void OnDestroy() override
	{
		//m_Mesh.Meshes.clear();
		//m_Mesh.Instances.clear();
		//m_Mesh.LoadedImages.clear();
		//m_Mesh.Textures.clear();
		//m_Mesh.Materials.clear();

		//m_Fence.Release();

		delete m_Renderer;
	}

private:
	std::vector<Yuki::GenericWindow*> m_Windows;
	Yuki::Fence m_Fence{};

	Yuki::Shader m_Shader{};
	Yuki::Pipeline m_Pipeline{};

	Yuki::CommandPool m_CommandPool{};
	Yuki::Buffer m_VertexBuffer{};

	Yuki::Unique<FreeCamera> m_Camera = nullptr;

	Yuki::SceneRenderer* m_Renderer = nullptr;

	Yuki::LoadedMesh m_Mesh;
};

YUKI_DECLARE_APPLICATION(TestApplication)
