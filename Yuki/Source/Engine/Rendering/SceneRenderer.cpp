#include "Rendering/SceneRenderer.hpp"
#include "Rendering/DescriptorSetBuilder.hpp"
#include "Rendering/PipelineBuilder.hpp"

namespace Yuki {

	SceneRenderer::SceneRenderer(RenderContext* InContext, SwapchainHandle InSwapchain)
		: m_Context(InContext)
	{
		m_TargetSwapchain = { InSwapchain, m_Context };
		m_GraphicsQueue = { m_Context->GetGraphicsQueue(), m_Context };

		m_StagingBuffer = Buffer(m_Context, {
			.Type = BufferType::StagingBuffer,
			.Size = 100 * 1024 * 1024,
		});

		m_MaterialsBuffer = Buffer(m_Context, {
			.Type = BufferType::StorageBuffer,
			.Size = sizeof(MeshMaterial) * 65536
		});
		
		m_Sampler = Sampler(m_Context);
		m_CommandPool = CommandPool(m_Context, m_GraphicsQueue);

		CreateDescriptorSets();
		CreatePipelines();
	}

	void SceneRenderer::RegisterMeshData(Mesh& InMesh)
	{
		LogInfo("Registering Mesh Data");

		for (auto& material : InMesh.Materials)
			material.AlbedoTextureIndex += m_TextureCount;

		uint32_t materialsSize = uint32_t(InMesh.Materials.size() * sizeof(MeshMaterial));
		m_Context->BufferSetData(m_StagingBuffer, InMesh.Materials.data(), materialsSize, 0);

		auto commandList = m_CommandPool.CreateCommandList();
		commandList.Begin();
		commandList.CopyToBuffer(m_MaterialsBuffer, m_MaterialCount * sizeof(MeshMaterial), m_StagingBuffer, 0, materialsSize);
		commandList.End();
		m_GraphicsQueue.SubmitCommandLists({ commandList }, {}, {});

		//InMesh.MaterialOffset = m_MaterialCount;

		m_MaterialSet.Write(0, InMesh.Textures, m_Sampler, m_TextureCount);
		m_MaterialCount += uint32_t(InMesh.Materials.size());
		m_TextureCount += uint32_t(InMesh.Textures.size());

		LogInfo("Done registering mesh data");
		LogInfo("Material Count: {}", m_MaterialCount);
		LogInfo("Texture Count: {}", m_TextureCount);
	}

	void SceneRenderer::BeginFrame(const Math::Mat4& InViewProjection)
	{
		m_PushConstants.ViewProjection = InViewProjection;

		m_CommandPool.Reset();

		m_CommandList = m_CommandPool.CreateCommandList();
		m_CommandList.Begin();
		m_CommandList.BindPipeline(m_ActivePipeline);
		m_CommandList.BindDescriptorSet(m_ActivePipeline, m_MaterialSet);

		// TODO(Peter): RenderTarget abstraction (Swapchain can be a render target)
		m_CommandList.BeginRendering(m_TargetSwapchain);

		m_PushConstants.MaterialVA = m_MaterialsBuffer.GetDeviceAddress();
	}

	void SceneRenderer::EndFrame(FenceHandle InFence)
	{
		m_CommandList.EndRendering();

		// Transition images to present
		m_CommandList.PrepareSwapchainPresent(m_TargetSwapchain);

		m_CommandList.End();
		m_GraphicsQueue.SubmitCommandLists({ m_CommandList }, { InFence }, { InFence });
	}

	void SceneRenderer::Submit(Mesh& InMesh)
	{
		//m_PushConstants.MaterialOffset = InMesh.MaterialOffset;

		for (const auto& meshInstance : InMesh.Instances)
		{
			const auto& meshData = InMesh.Sources[meshInstance.SourceIndex];
			m_PushConstants.Transform = meshInstance.Transform;
			m_PushConstants.VertexVA = meshData.VertexData.GetDeviceAddress();
			m_CommandList.PushConstants(m_ActivePipeline, &m_PushConstants, sizeof(PushConstants));
			m_CommandList.BindIndexBuffer(meshData.IndexBuffer, 0);
			m_CommandList.DrawIndexed(meshData.IndexCount);
		}
	}

	void SceneRenderer::CreateDescriptorSets()
	{
		DescriptorCount descriptorPoolCounts[] =
		{
			{ DescriptorType::CombinedImageSampler, 65536 },
		};
		m_DescriptorPool = DescriptorPool(m_Context, descriptorPoolCounts);

		m_DescriptorSetLayout = DescriptorSetLayoutBuilder(m_Context)
			.Stages(ShaderStage::Vertex | ShaderStage::Fragment)
			.Binding(65536, DescriptorType::CombinedImageSampler)
			.Build();

		m_MaterialSet = m_DescriptorPool.AllocateDescriptorSet(m_DescriptorSetLayout);
	}

	void SceneRenderer::CreatePipelines()
	{
		m_MeshShader = Shader(m_Context, std::filesystem::path("Resources/Shaders/Geometry.glsl"));
		
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
