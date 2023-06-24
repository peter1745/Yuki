#include "Rendering/SceneRenderer.hpp"
#include "Rendering/DescriptorSetBuilder.hpp"
#include "Rendering/PipelineBuilder.hpp"

namespace Yuki {

	SceneRenderer::SceneRenderer(RenderContext* InContext, Swapchain InSwapchain)
		: m_Context(InContext), m_TargetSwapchain(InSwapchain)
	{
		m_GraphicsQueue = m_Context->GetGraphicsQueue();

		m_StagingBuffer = m_Context->CreateBuffer({
			.Type = Yuki::BufferType::StagingBuffer,
			.Size = 100 * 1024 * 1024,
		});

		m_MaterialsBuffer = m_Context->CreateBuffer({
			.Type = Yuki::BufferType::StorageBuffer,
			.Size = sizeof(Yuki::MeshMaterial) * 65536
		});

		m_Sampler = m_Context->CreateSampler();
		m_CommandPool = m_Context->CreateCommandPool(m_GraphicsQueue);

		CreateDescriptorSets();
		CreatePipelines();
	}

	void SceneRenderer::BeginFrame(const Math::Mat4& InViewProjection)
	{
		m_PushConstants.ViewProjection = InViewProjection;

		m_Context->CommandPoolReset(m_CommandPool);

		m_CommandList = m_Context->CreateCommandList(m_CommandPool);
		m_Context->CommandListBegin(m_CommandList);
		m_Context->CommandListBindPipeline(m_CommandList, m_ActivePipeline);
		//m_Context->CommandListBindDescriptorSet(m_CommandList, m_ActivePipeline, m_MaterialSet);

		// TODO(Peter): RenderTarget abstraction (Swapchain can be a render target)
		m_Context->CommandListBeginRendering(m_CommandList, m_TargetSwapchain);
	}

	void SceneRenderer::EndFrame(Fence InFence)
	{
		m_Context->CommandListEndRendering(m_CommandList);
		m_Context->CommandListEnd(m_CommandList);
		m_Context->QueueSubmitCommandLists(m_GraphicsQueue, { m_CommandList }, { InFence }, {});
	}

#if 0
	void SceneRenderer::Submit(LoadedMesh& InMesh)
	{
		if (InMesh.Textures.empty())
		{
			InMesh.Textures.resize(InMesh.LoadedImages.size());
			for (size_t i = 0; i < InMesh.LoadedImages.size(); i++)
			{
				auto commandList = m_Context->CreateCommandList(m_CommandPool);
				m_Context->CommandListBegin(commandList);

				const auto& imageData = InMesh.LoadedImages[i];

				Yuki::Image blittedImage{};
				bool blitted = false;
				Yuki::Image image = m_Context->CreateImage(imageData.Width, imageData.Height, Yuki::ImageFormat::RGBA8UNorm, Yuki::ImageUsage::Sampled | Yuki::ImageUsage::TransferSource | Yuki::ImageUsage::TransferDestination);
				m_Context->CommandListTransitionImage(commandList, image, Yuki::ImageLayout::ShaderReadOnly);
				m_Context->BufferSetData(m_StagingBuffer, imageData.Data.data(), uint32_t(imageData.Data.size()));
				m_Context->CommandListCopyToImage(commandList, image, m_StagingBuffer, 0);

				if (imageData.Width > 1024 && imageData.Height > 1024)
				{
					blittedImage = m_Context->CreateImage(1024, 1024, Yuki::ImageFormat::RGBA8UNorm, Yuki::ImageUsage::Sampled | Yuki::ImageUsage::TransferDestination);
					m_Context->CommandListTransitionImage(commandList, blittedImage, Yuki::ImageLayout::ShaderReadOnly);
					m_Context->CommandListBlitImage(commandList, blittedImage, image);
					blitted = true;
				}

				m_Context->CommandListEnd(commandList);
				m_Context->QueueSubmitCommandLists(m_GraphicsQueue, { commandList }, {}, {});
				m_Context->DeviceWaitIdle();

				if (blitted)
				{
					m_Context->Destroy(image);
					image = blittedImage;
				}

				InMesh.Textures[i] = image;
			}

			m_Context->DescriptorSetWrite(m_MaterialSet, 1, InMesh.Textures, m_Sampler);
		}

		for (auto& meshSource : InMesh.Meshes)
		{
			if (meshSource.VertexData != Buffer{} && meshSource.IndexBuffer != Buffer{})
				break;

			meshSource.VertexData = m_Context->CreateBuffer({
				.Type = BufferType::StorageBuffer,
				.Size = uint32_t(sizeof(Vertex) * meshSource.Vertices.size())
			});

			{
				m_Context->BufferSetData(m_StagingBuffer, meshSource.Vertices.data(), uint32_t(sizeof(Vertex) * meshSource.Vertices.size()));

				auto commandList = m_Context->CreateCommandList(m_CommandPool);
				m_Context->CommandListBegin(commandList);
				m_Context->CommandListCopyToBuffer(commandList, meshSource.VertexData, 0, m_StagingBuffer, 0, 0);
				m_Context->CommandListEnd(commandList);
				m_Context->QueueSubmitCommandLists(m_GraphicsQueue, { commandList }, {}, {});
				m_Context->DeviceWaitIdle();
			}

			meshSource.IndexBuffer = m_Context->CreateBuffer({
				.Type = BufferType::IndexBuffer,
				.Size = uint32_t(sizeof(uint32_t) * meshSource.Indices.size())
			});

			{
				m_Context->BufferSetData(m_StagingBuffer, meshSource.Indices.data(), uint32_t(sizeof(uint32_t) * meshSource.Indices.size()));

				auto commandList = m_Context->CreateCommandList(m_CommandPool);
				m_Context->CommandListBegin(commandList);
				m_Context->CommandListCopyToBuffer(commandList, meshSource.IndexBuffer, 0, m_StagingBuffer, 0, 0);
				m_Context->CommandListEnd(commandList);
				m_Context->QueueSubmitCommandLists(m_GraphicsQueue, { commandList }, {}, {});
				m_Context->DeviceWaitIdle();
			}
		}

		{
			auto commandList = m_Context->CreateCommandList(m_CommandPool);
			m_Context->CommandListBegin(commandList);
			m_Context->BufferSetData(m_StagingBuffer, InMesh.Materials.data(), uint32_t(InMesh.Materials.size() * sizeof(MeshMaterial)));
			m_Context->CommandListCopyToBuffer(commandList, m_MaterialsBuffer, 0, m_StagingBuffer, 0, 0);
			m_Context->CommandListEnd(commandList);
			m_Context->QueueSubmitCommandLists(m_GraphicsQueue, { commandList }, {}, {});
			m_Context->DeviceWaitIdle();

			std::array<std::pair<uint32_t, Yuki::Buffer>, 1> bufferArray{std::pair{ 0, m_MaterialsBuffer }};
			m_Context->DescriptorSetWrite(m_MaterialSet, 0, bufferArray);
		}

		for (const auto& meshInstance : InMesh.Instances)
		{
			m_PushConstants.Transform = meshInstance.Transform;
			m_PushConstants.VertexVA = m_Context->BufferGetDeviceAddress(meshInstance.SourceMesh->VertexData);
			m_Context->CommandListPushConstants(m_CommandList, m_ActivePipeline, &m_PushConstants, sizeof(PushConstants));
			m_Context->CommandListBindBuffer(m_CommandList, meshInstance.SourceMesh->IndexBuffer);
			m_Context->CommandListDrawIndexed(m_CommandList, uint32_t(meshInstance.SourceMesh->Indices.size()));
		}
	}
#else
void SceneRenderer::Submit(const Mesh& InMesh)
	{
		for (const auto& meshInstance : InMesh.Instances)
		{
			const auto& meshData = InMesh.Sources[meshInstance.SourceIndex];
			m_PushConstants.Transform = meshInstance.Transform;
			m_PushConstants.VertexVA = m_Context->BufferGetDeviceAddress(meshData.VertexData);
			m_Context->CommandListPushConstants(m_CommandList, m_ActivePipeline, &m_PushConstants, sizeof(PushConstants));
			m_Context->CommandListBindBuffer(m_CommandList, meshData.IndexBuffer);
			m_Context->CommandListDrawIndexed(m_CommandList, meshData.IndexCount);
		}
	}
#endif

	void SceneRenderer::CreateDescriptorSets()
	{
		Yuki::DescriptorCount descriptorPoolCounts[] =
		{
			{ Yuki::DescriptorType::CombinedImageSampler, 65536 },
			{ Yuki::DescriptorType::StorageBuffer, 65536 },
		};
		m_DescriptorPool = m_Context->CreateDescriptorPool(descriptorPoolCounts);

		m_DescriptorSetLayout = DescriptorSetLayoutBuilder(m_Context)
			.Stages(ShaderStage::Vertex | ShaderStage::Fragment)
			.Binding(65536, DescriptorType::StorageBuffer)
			.Binding(256, DescriptorType::CombinedImageSampler)
			.Build();

		m_MaterialSet = m_Context->DescriptorPoolAllocateDescriptorSet(m_DescriptorPool, m_DescriptorSetLayout);
	}

	void SceneRenderer::CreatePipelines()
	{
		m_MeshShader = m_Context->CreateShader("Resources/Shaders/Geometry.glsl");
		
		m_Pipeline = Yuki::PipelineBuilder(m_Context)
			.WithShader(m_MeshShader)
			.PushConstant(sizeof(PushConstants))
			//.AddDescriptorSetLayout(m_DescriptorSetLayout)
			.ColorAttachment(Yuki::ImageFormat::BGRA8UNorm)
			.DepthAttachment()
			.Build();

		m_WireframePipeline = Yuki::PipelineBuilder(m_Context)
			.WithShader(m_MeshShader)
			.PushConstant(sizeof(PushConstants))
			.AddDescriptorSetLayout(m_DescriptorSetLayout)
			.ColorAttachment(Yuki::ImageFormat::BGRA8UNorm)
			.DepthAttachment()
			.SetPolygonMode(PolygonModeType::Line)
			.Build();

		m_ActivePipeline = m_Pipeline;
	}

}
