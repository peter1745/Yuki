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

		m_CommandPool = GetRenderContext()->CreateCommandPool();

		//m_Renderer = new Yuki::SceneRenderer(GetRenderContext());

		m_Sampler = GetRenderContext()->CreateSampler();

		Yuki::DescriptorCount descriptorPoolCounts[] =
		{
			{ Yuki::DescriptorType::CombinedImageSampler, 65536 },
			{ Yuki::DescriptorType::StorageBuffer, 65536 },
		};
		m_DescriptorPool = GetRenderContext()->CreateDescriptorPool(descriptorPoolCounts);
		
		auto materialSetLayout = Yuki::DescriptorSetLayoutBuilder(GetRenderContext())
				.Stages(Yuki::ShaderStage::Vertex | Yuki::ShaderStage::Fragment)
				.Binding(65536, Yuki::DescriptorType::StorageBuffer)
				.Binding(256, Yuki::DescriptorType::CombinedImageSampler)
				.Build();
		m_MaterialDescriptorSet = GetRenderContext()->DescriptorPoolAllocateDescriptorSet(m_DescriptorPool, materialSetLayout);

		m_Mesh = Yuki::MeshLoader::LoadGLTFMesh(GetRenderContext(), "Resources/Meshes/deccer-cubes/SM_Deccer_Cubes_Textured_Complex.gltf");
		//m_Mesh = Yuki::MeshLoader::LoadGLTFMesh(GetRenderContext(), "Resources/Meshes/NewSponza_Main_glTF_002.gltf");

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

			GetRenderContext()->DescriptorSetWrite(m_MaterialDescriptorSet, 1, m_Mesh.Textures, m_Sampler);
		}

		m_Shader = GetRenderContext()->CreateShader("Resources/Shaders/Geometry.glsl");

		
		m_MaterialsBuffer = GetRenderContext()->CreateBuffer({
			.Type = Yuki::BufferType::StorageBuffer,
			.Size = sizeof(Yuki::MeshMaterial) * 65536
		});

		{
			GetRenderContext()->BufferSetData(stagingBuffer, m_Mesh.Materials.data(), uint32_t(m_Mesh.Materials.size() * sizeof(Yuki::MeshMaterial)));

			auto commandList = GetRenderContext()->CreateCommandList(m_CommandPool);
			GetRenderContext()->CommandListBegin(commandList);
			GetRenderContext()->CommandListCopyToBuffer(commandList, m_MaterialsBuffer, 0, stagingBuffer, 0, 0);
			GetRenderContext()->CommandListEnd(commandList);
			GetRenderContext()->QueueSubmitCommandLists({ commandList }, {}, {});
			GetRenderContext()->DeviceWaitIdle();

			std::array<std::pair<uint32_t, Yuki::Buffer>, 1> bufferArray{std::pair{ 0, m_MaterialsBuffer }};
			GetRenderContext()->DescriptorSetWrite(m_MaterialDescriptorSet, 0, bufferArray);
		}

		/*
		TODO(Peter):
			- Implement Proxy Objects over the direct RenderContext interface
			- Multiple frames in flight
			- Make sure multiple windows / swapchains still works
		*/

		m_Pipeline = Yuki::PipelineBuilder(GetRenderContext())
			.WithShader(m_Shader)
			.AddVertexInput(Yuki::ShaderDataType::Float3)
			.AddVertexInput(Yuki::ShaderDataType::Float3)
			.AddVertexInput(Yuki::ShaderDataType::Float2)
			.AddVertexInput(Yuki::ShaderDataType::UInt)
			.PushConstant(sizeof(FrameTransforms))
			.AddDescriptorSetLayout(materialSetLayout)
			.ColorAttachment(Yuki::ImageFormat::BGRA8UNorm)
			.DepthAttachment()
			.Build();

		for (auto& meshSource : m_Mesh.Meshes)
		{
			meshSource.VertexBuffer = GetRenderContext()->CreateBuffer({
				.Type = Yuki::BufferType::VertexBuffer,
				.Size = uint32_t(sizeof(Yuki::Vertex) * meshSource.Vertices.size())
			});

			{
				GetRenderContext()->BufferSetData(stagingBuffer, meshSource.Vertices.data(), uint32_t(sizeof(Yuki::Vertex) * meshSource.Vertices.size()));

				auto commandList = GetRenderContext()->CreateCommandList(m_CommandPool);
				GetRenderContext()->CommandListBegin(commandList);
				GetRenderContext()->CommandListCopyToBuffer(commandList, meshSource.VertexBuffer, 0, stagingBuffer, 0, 0);
				GetRenderContext()->CommandListEnd(commandList);
				GetRenderContext()->QueueSubmitCommandLists({ commandList }, {}, {});
				GetRenderContext()->DeviceWaitIdle();
			}

			meshSource.IndexBuffer = GetRenderContext()->CreateBuffer({
				.Type = Yuki::BufferType::IndexBuffer,
				.Size = uint32_t(sizeof(uint32_t) * meshSource.Indices.size())
			});

			{
				GetRenderContext()->BufferSetData(stagingBuffer, meshSource.Indices.data(), uint32_t(sizeof(uint32_t) * meshSource.Indices.size()));

				auto commandList = GetRenderContext()->CreateCommandList(m_CommandPool);
				GetRenderContext()->CommandListBegin(commandList);
				GetRenderContext()->CommandListCopyToBuffer(commandList, meshSource.IndexBuffer, 0, stagingBuffer, 0, 0);
				GetRenderContext()->CommandListEnd(commandList);
				GetRenderContext()->QueueSubmitCommandLists({ commandList }, {}, {});
				GetRenderContext()->DeviceWaitIdle();
			}
		}

		m_Camera = Yuki::Unique<FreeCamera>::Create(m_Windows[0]);
	}

	void OnRunLoop() override
	{
		m_Camera->Update(0.0f);

		// Collect Swapchains
		auto swapchains = GetRenderContext()->GetSwapchains();

		if (swapchains.empty())
			return;

		GetRenderContext()->FenceWait(m_Fence);

		// Acquire Images for all Viewports
		GetRenderContext()->QueueAcquireImages(swapchains, { m_Fence });

		auto commandList = GetRenderContext()->CreateCommandList(m_CommandPool);
		GetRenderContext()->CommandListBegin(commandList);
		GetRenderContext()->CommandListBindPipeline(commandList, m_Pipeline);
		GetRenderContext()->CommandListBindDescriptorSet(commandList, m_Pipeline, m_MaterialDescriptorSet);
		GetRenderContext()->CommandListBeginRendering(commandList, swapchains[0]);

		m_FrameTransforms.ViewProjection = Yuki::Math::Mat4::PerspectiveInfReversedZ(70.0f, 1920.0f / 1080.0f, 0.05f) * m_Camera->GetViewMatrix();

		for (const auto& meshInstance : m_Mesh.Instances)
		{
			m_FrameTransforms.Transform = meshInstance.Transform;
			GetRenderContext()->CommandListPushConstants(commandList, m_Pipeline, &m_FrameTransforms, sizeof(FrameTransforms));
			GetRenderContext()->CommandListBindBuffer(commandList, meshInstance.SourceMesh->VertexBuffer);
			GetRenderContext()->CommandListBindBuffer(commandList, meshInstance.SourceMesh->IndexBuffer);
			GetRenderContext()->CommandListDrawIndexed(commandList, meshInstance.SourceMesh->Indices.size());
		}

		GetRenderContext()->CommandListEndRendering(commandList);
		GetRenderContext()->CommandListEnd(commandList);
		GetRenderContext()->QueueSubmitCommandLists({ commandList}, { m_Fence }, {});

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

	Yuki::Shader m_Shader{};
	Yuki::Pipeline m_Pipeline{};

	Yuki::CommandPool m_CommandPool{};
	Yuki::Buffer m_VertexBuffer{};
	Yuki::Buffer m_IndexBuffer{};

	Yuki::Buffer m_MaterialsBuffer{};

	Yuki::Sampler m_Sampler{};

	Yuki::DescriptorPool m_DescriptorPool{};
	Yuki::DescriptorSetLayout m_DescriptorSetLayout{};
	Yuki::DescriptorSet m_MaterialDescriptorSet{};

	Yuki::Unique<FreeCamera> m_Camera = nullptr;

	Yuki::SceneRenderer* m_Renderer = nullptr;

	Yuki::LoadedMesh m_Mesh;

	struct FrameTransforms
	{
		Yuki::Math::Mat4 ViewProjection;
		Yuki::Math::Mat4 Transform;
	} m_FrameTransforms;
};

YUKI_DECLARE_APPLICATION(TestApplication)
