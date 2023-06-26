#include "Rendering/SceneRenderer.hpp"
#include "Rendering/DescriptorSetBuilder.hpp"
#include "Rendering/PipelineBuilder.hpp"

namespace Yuki {

	SceneRenderer::SceneRenderer(RenderContext* InContext, Swapchain InSwapchain)
		: m_Context(InContext), m_TargetSwapchain(InSwapchain)
	{
		m_GraphicsQueue = m_Context->GetGraphicsQueue();

		m_StagingBuffer = m_Context->CreateBuffer({
			.Type = BufferType::StagingBuffer,
			.Size = 100 * 1024 * 1024,
		});

		m_MaterialsBuffer = m_Context->CreateBuffer({
			.Type = BufferType::StorageBuffer,
			.Size = sizeof(MeshMaterial) * 65536
		});
		
		m_Sampler = m_Context->CreateSampler();
		m_CommandPool = m_Context->CreateCommandPool(m_GraphicsQueue);

		CreateDescriptorSets();
		CreatePipelines();
	}

	void SceneRenderer::RegisterMeshData(Mesh& InMesh)
	{
		for (auto& material : InMesh.Materials)
			material.AlbedoTextureIndex += m_TextureCount;

		uint32_t materialsSize = uint32_t(InMesh.Materials.size() * sizeof(MeshMaterial));
		m_Context->BufferSetData(m_StagingBuffer, InMesh.Materials.data(), materialsSize, 0);

		auto commandList = m_Context->CreateCommandList(m_CommandPool);
		m_Context->CommandListBegin(commandList);
		m_Context->CommandListCopyToBuffer(commandList, m_MaterialsBuffer, m_MaterialCount * sizeof(MeshMaterial), m_StagingBuffer, 0, materialsSize);
		m_Context->CommandListEnd(commandList);
		m_Context->QueueSubmitCommandLists(m_GraphicsQueue, { commandList }, {}, {});

		InMesh.MaterialOffset = m_MaterialCount;

		m_Context->DescriptorSetWrite(m_MaterialSet, 0, InMesh.Textures, m_Sampler, m_TextureCount);
		m_MaterialCount += uint32_t(InMesh.Materials.size());
		m_TextureCount += uint32_t(InMesh.Textures.size());

		LogInfo("Material Count: {}", m_MaterialCount);
		LogInfo("Texture Count: {}", m_TextureCount);
	}

	void SceneRenderer::BeginFrame(const Math::Mat4& InViewProjection)
	{
		m_PushConstants.ViewProjection = InViewProjection;

		m_Context->CommandPoolReset(m_CommandPool);

		m_CommandList = m_Context->CreateCommandList(m_CommandPool);
		m_Context->CommandListBegin(m_CommandList);
		m_Context->CommandListBindPipeline(m_CommandList, m_ActivePipeline);
		m_Context->CommandListBindDescriptorSet(m_CommandList, m_ActivePipeline, m_MaterialSet);

		// TODO(Peter): RenderTarget abstraction (Swapchain can be a render target)
		m_Context->CommandListBeginRendering(m_CommandList, m_TargetSwapchain);

		m_PushConstants.MaterialVA = m_Context->BufferGetDeviceAddress(m_MaterialsBuffer);
	}

	void SceneRenderer::EndFrame(Fence InFence)
	{
		m_Context->CommandListEndRendering(m_CommandList);

		// Transition images to present
		m_Context->CommandListPrepareSwapchainPresent(m_CommandList, m_TargetSwapchain);

		m_Context->CommandListEnd(m_CommandList);
		m_Context->QueueSubmitCommandLists(m_GraphicsQueue, { m_CommandList }, { InFence }, { InFence });
	}

	void SceneRenderer::Submit(Mesh& InMesh)
	{
		m_PushConstants.MaterialOffset = InMesh.MaterialOffset;

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

	void SceneRenderer::CreateDescriptorSets()
	{
		DescriptorCount descriptorPoolCounts[] =
		{
			{ DescriptorType::CombinedImageSampler, 65536 },
		};
		m_DescriptorPool = m_Context->CreateDescriptorPool(descriptorPoolCounts);

		m_DescriptorSetLayout = DescriptorSetLayoutBuilder(m_Context)
			.Stages(ShaderStage::Vertex | ShaderStage::Fragment)
			.Binding(65536, DescriptorType::CombinedImageSampler)
			.Build();

		m_MaterialSet = m_Context->DescriptorPoolAllocateDescriptorSet(m_DescriptorPool, m_DescriptorSetLayout);
	}

	void SceneRenderer::CreatePipelines()
	{
		m_MeshShader = m_Context->CreateShader("Resources/Shaders/Geometry.glsl");
		
		m_Pipeline = PipelineBuilder(m_Context)
			.WithShader(m_MeshShader)
			.PushConstant(sizeof(PushConstants))
			.AddDescriptorSetLayout(m_DescriptorSetLayout)
			.ColorAttachment(ImageFormat::BGRA8UNorm)
			.DepthAttachment()
			.Build();

		m_WireframePipeline = PipelineBuilder(m_Context)
			.WithShader(m_MeshShader)
			.PushConstant(sizeof(PushConstants))
			.AddDescriptorSetLayout(m_DescriptorSetLayout)
			.ColorAttachment(ImageFormat::BGRA8UNorm)
			.DepthAttachment()
			.SetPolygonMode(PolygonModeType::Line)
			.Build();

		m_ActivePipeline = m_Pipeline;
	}

}
