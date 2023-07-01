#include "Rendering/EntityRenderer.hpp"
#include "Rendering/RenderContext.hpp"
#include "Rendering/DescriptorSetBuilder.hpp"
#include "Entities/TransformComponents.hpp"

namespace Yuki {

	EntityRenderer::EntityRenderer(RenderContext* InContext, SwapchainHandle InSwapchain, flecs::world& InWorld)
		: m_Context(InContext), m_World(InWorld), m_Swapchain(InSwapchain)
	{
		m_GraphicsQueue = { m_Context->GetGraphicsQueue(), m_Context };

		PreRenderPhase = m_World.entity().add(flecs::Phase).depends_on(flecs::PostUpdate);
		RenderPhase = m_World.entity().add(flecs::Phase).depends_on(PreRenderPhase);
		PostRenderPhase = m_World.entity().add(flecs::Phase).depends_on(RenderPhase);

		m_World.observer<Entities::MeshComponent>()
			.event(flecs::OnSet)
			.each([this](flecs::iter& InIter, size_t InEntityIndex, Entities::MeshComponent& InMesh)
			{
				auto& mesh = m_Meshes.Get(InMesh.Value);

				uint32_t bufferIndex = m_LastInstanceID;

				for (auto& meshInstance : mesh.Instances)
				{
					GPUObject gpuObject =
					{
						.VertexVA = mesh.Sources[meshInstance.SourceIndex].VertexData.GetDeviceAddress(),
						.MaterialVA = mesh.MaterialStorageBuffer.GetDeviceAddress()
					};

					m_ObjectStagingBuffer.SetData(&gpuObject, sizeof(GPUObject), m_LastInstanceID * sizeof(GPUObject));
					m_TransformStagingBuffer.SetData(&meshInstance.Transform, sizeof(Math::Mat4), m_LastInstanceID * sizeof(Math::Mat4));
					m_LastInstanceID++;
				}

				auto commandList = m_CommandPool.CreateCommandList();
				commandList.Begin();
				commandList.CopyToBuffer(m_ObjectStorageBuffer, bufferIndex * sizeof(GPUObject), m_ObjectStagingBuffer, bufferIndex * sizeof(GPUObject), uint32_t(mesh.Instances.size()) * sizeof(GPUObject));
				commandList.CopyToBuffer(m_TransformStorageBuffer, bufferIndex * sizeof(Math::Mat4), m_TransformStagingBuffer, bufferIndex * sizeof(Math::Mat4), uint32_t(mesh.Instances.size()) * sizeof(Math::Mat4));
				commandList.End();

				m_GraphicsQueue.SubmitCommandLists({ commandList }, { m_Fence }, { m_Fence });
			});


		m_CommandPool = CommandPool(m_Context, m_GraphicsQueue);

		m_TransformStorageBuffer = Buffer(m_Context, {
			.Type = BufferType::StorageBuffer,
			.Size = 655360 * sizeof(Entities::LocalTransform)
		});

		m_TransformStagingBuffer = Buffer(m_Context, {
			.Type = BufferType::StagingBuffer,
			.Size = 100 * 1024 * 1024
		});

		m_ObjectStorageBuffer = Buffer(m_Context, {
			.Type = BufferType::StorageBuffer,
			.Size = 655360 * sizeof(GPUObject)
		});

		m_ObjectStagingBuffer = Buffer(m_Context, {
			.Type = BufferType::StagingBuffer,
			.Size = 100 * 1024 * 1024
		});

		m_Sampler = Sampler(m_Context);

		DescriptorCount descriptorPoolCounts[] =
		{
			{ Yuki::DescriptorType::CombinedImageSampler, 65536 },
		};
		m_DescriptorPool = Yuki::DescriptorPool(m_Context, descriptorPoolCounts);

		m_DescriptorSetLayout = DescriptorSetLayoutBuilder(m_Context)
			.Stages(ShaderStage::Vertex | ShaderStage::Fragment)
			.Binding(65536, DescriptorType::CombinedImageSampler)
			.Build();

		m_MaterialSet = m_DescriptorPool.AllocateDescriptorSet(m_DescriptorSetLayout);

		m_Shader = Shader(m_Context, std::filesystem::path("Resources/Shaders/Geometry.glsl"));
		
		m_Pipeline = Yuki::PipelineBuilder(m_Context)
			.WithShader(m_Shader)
			.PushConstant(sizeof(PushConstants))
			.AddDescriptorSetLayout(m_DescriptorSetLayout)
			.ColorAttachment(Yuki::ImageFormat::BGRA8UNorm)
			.DepthAttachment()
			.Build();

		m_Fence = Fence(m_Context);
	}

	MeshHandle EntityRenderer::SubmitForUpload(Mesh InMesh)
	{
		auto[handle, mesh] = m_Meshes.Insert(InMesh);
		
		std::scoped_lock lock(m_MeshUploadMutex);
		m_MeshUploadQueue.push_back(handle);
		YUKI_UNUSED(mesh);

		return handle;
	}

	void EntityRenderer::Reset()
	{
		m_CommandPool.Reset();
	}

	void EntityRenderer::PrepareFrame()
	{
		std::scoped_lock lock(m_MeshUploadMutex);

		while (!m_MeshUploadQueue.empty())
		{
			auto& mesh = m_Meshes.Get(m_MeshUploadQueue.back());

			m_MaterialSet.Write(0, mesh.Textures, m_Sampler, m_TextureCount);
			m_TextureCount += uint32_t(mesh.Textures.size());

			m_MeshUploadQueue.pop_back();
		}
	}

	void EntityRenderer::BeginFrame(const Math::Mat4& InViewProjection)
	{
		PrepareFrame();

		m_PushConstants.ViewProjection = InViewProjection;
		m_PushConstants.ObjectVA = m_ObjectStorageBuffer.GetDeviceAddress();
		m_PushConstants.TransformVA = m_TransformStorageBuffer.GetDeviceAddress();

		m_CommandList = m_CommandPool.CreateCommandList();
		m_CommandList.Begin();
		
		m_CommandList.BindPipeline(m_Pipeline);
		m_CommandList.BindDescriptorSet(m_Pipeline, m_MaterialSet);
		m_CommandList.PushConstants(m_Pipeline, &m_PushConstants, sizeof(m_PushConstants));
		m_CommandList.BeginRendering(m_Swapchain);
	}

	void EntityRenderer::RenderEntities()
	{
		uint32_t instanceIndex = 0;

		auto filter = m_World.filter<const Entities::MeshComponent>();
		filter.each([&](const Entities::MeshComponent& InMesh)
		{
			const auto& mesh = m_Meshes.Get(InMesh.Value);

			for (const auto& meshInstance : mesh.Instances)
			{
				const auto& meshData = mesh.Sources[meshInstance.SourceIndex];
				m_CommandList.BindIndexBuffer(meshData.IndexBuffer, 0);
				m_CommandList.DrawIndexed(meshData.IndexCount, 0, instanceIndex);
				instanceIndex++;
			}
		});

	}

	void EntityRenderer::EndFrame()
	{
		m_CommandList.EndRendering();
		m_CommandList.End();
		m_GraphicsQueue.SubmitCommandLists({ m_CommandList }, { m_Fence }, { m_Fence });
	}

}
