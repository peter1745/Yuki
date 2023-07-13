#include "Rendering/EntityRenderer.hpp"
#include "Rendering/RenderContext.hpp"
#include "Rendering/DescriptorSetBuilder.hpp"
#include "Entities/TransformComponents.hpp"
#include "Math/Math.hpp"

namespace Yuki {

	WorldRenderer::WorldRenderer(World& InWorld, RenderContext* InContext)
		: Renderer(InContext), m_World(InWorld)
	{
		m_GraphicsQueue = { m_Context->GetGraphicsQueue(), m_Context };

		m_CommandPool = CommandPool(m_Context, m_GraphicsQueue);

		m_TransformStorageBuffer = Buffer(m_Context, {
			.Type = BufferType::StorageBuffer,
			.Size = 655360 * sizeof(Entities::LocalTransform)
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
		
		m_Pipeline = PipelineBuilder(m_Context)
			.WithShader(m_Shader)
			.PushConstant(sizeof(PushConstants))
			.AddDescriptorSetLayout(m_DescriptorSetLayout)
			.ColorAttachment(ImageFormat::RGBA8UNorm)
			.DepthAttachment()
			.Build();

		m_WireframePipeline = PipelineBuilder(m_Context)
			.WithShader(m_Shader)
			.PushConstant(sizeof(PushConstants))
			.AddDescriptorSetLayout(m_DescriptorSetLayout)
			.ColorAttachment(ImageFormat::RGBA8UNorm)
			.DepthAttachment()
			.SetPolygonMode(PolygonModeType::Line)
			.Build();

		m_Fence = Fence(m_Context);

		m_ColorImage = Image(InContext, 1920, 1080, ImageFormat::RGBA8UNorm, ImageUsage::ColorAttachment | ImageUsage::Sampled);
		m_DepthImage = Image(InContext, 1920, 1080, ImageFormat::Depth32SFloat, ImageUsage::DepthAttachment | ImageUsage::Sampled);

		m_ViewportWidth = 1920;
		m_ViewportHeight = 1080;
	}

	void WorldRenderer::CreateGPUInstance(flecs::entity InRoot)
	{
		ScheduleTransfer([this, InRoot](Queue InQueue, CommandPool InPool, Buffer InStagingBuffer, Fence InFence)
		{
			uint32_t baseObjectIndex = m_GPUObjectCount.load();
			uint32_t objectIndex = baseObjectIndex;

			m_World.IterateHierarchy(InRoot, [this, &InStagingBuffer, &objectIndex, baseObjectIndex](flecs::entity InEntity)
			{
				if (!InEntity.has<Entities::MeshComponent>())
					return;

				const auto* translation = InEntity.get<Entities::Translation>();
				const auto* rotation = InEntity.get<Entities::Rotation>();
				const auto* scale = InEntity.get<Entities::Scale>();
				const auto* meshComponent = InEntity.get<Entities::MeshComponent>();
				const auto& mesh = m_GPUMeshScenes.at(meshComponent->MeshID);
				InEntity.get_mut<Entities::GPUTransform>()->BufferIndex = objectIndex;

				mesh.UploadBarrier.Wait();

				const auto& gpuMesh = mesh.Meshes[meshComponent->MeshIndex];

				GPUObject gpuObject =
				{
					.VertexVA = gpuMesh.VertexData.GetDeviceAddress(),
					.MaterialVA = mesh.MaterialData.GetDeviceAddress(),
					.BaseTextureOffset = mesh.BaseTextureOffset
				};

				Math::Mat4 transform = Math::Mat4::Translation(translation->Value) * Math::Mat4::Rotation(rotation->Value) * Math::Mat4::Scale(scale->Value);
				m_ObjectStagingBuffer.SetData(&gpuObject, sizeof(GPUObject), objectIndex * sizeof(GPUObject));
				InStagingBuffer.SetData(&transform, sizeof(Math::Mat4), (objectIndex - baseObjectIndex) * sizeof(Math::Mat4));
				objectIndex++;
			});

			auto commandList = InPool.CreateCommandList();
			commandList.Begin();
			commandList.CopyToBuffer(m_ObjectStorageBuffer, baseObjectIndex * sizeof(GPUObject), m_ObjectStagingBuffer, baseObjectIndex * sizeof(GPUObject), objectIndex * sizeof(GPUObject));
			commandList.CopyToBuffer(m_TransformStorageBuffer, baseObjectIndex * sizeof(Math::Mat4), InStagingBuffer, 0,  (objectIndex - baseObjectIndex) * sizeof(Math::Mat4));
			commandList.End();

			InQueue.SubmitCommandLists({ commandList }, { InFence }, { InFence });

			m_GPUObjectCount += objectIndex;
		});
	}

	void WorldRenderer::SubmitForUpload(AssetID InAssetID, const MeshScene& InMeshScene)
	{
		static constexpr uint32_t CopyChunkSize = 100 * 1024 * 1024;

		if (m_GPUMeshScenes.contains(InAssetID))
			return;

		auto& gpuMeshScene = m_GPUMeshScenes[InAssetID];
		gpuMeshScene.Meshes.resize(InMeshScene.Meshes.size());
		gpuMeshScene.MaterialData = Buffer(m_Context, {
			.Type = BufferType::StorageBuffer,
			.Size = uint32_t(InMeshScene.Materials.size() * sizeof(MaterialData))
		});

		ScheduleTransfer([this, &gpuMeshScene, InMeshScene](Queue InQueue, CommandPool InPool, Buffer InStagingBuffer, Fence InFence)
		{
			Fence fence{m_Context};

			uint32_t materialBufferSize = uint32_t(InMeshScene.Materials.size()) * sizeof(MaterialData);

			InStagingBuffer.SetData(InMeshScene.Materials.data(), materialBufferSize, 0);

			auto commandList = InPool.CreateCommandList();
			commandList.Begin();
			commandList.CopyToBuffer(gpuMeshScene.MaterialData, 0, InStagingBuffer, 0, materialBufferSize);
			commandList.End();

			InQueue.SubmitCommandLists({ commandList }, {}, { fence });
			fence.Wait();

			m_Context->Destroy(fence);
		}, &gpuMeshScene.UploadBarrier);

		ScheduleTransfer([this, &gpuMeshScene, InMeshScene](Queue InQueue, CommandPool InPool, Buffer InStagingBuffer, Fence InFence)
		{
			Fence fence{ m_Context };

			for (size_t i = 0; i < InMeshScene.Meshes.size(); i++)
			{
				const auto& mesh = InMeshScene.Meshes[i];
				auto& gpuMesh = gpuMeshScene.Meshes[i];

				{
					uint32_t vertexDataSize = uint32_t(mesh.Vertices.size()) * sizeof(Vertex);

					gpuMesh.VertexData = Buffer(m_Context, {
						.Type = BufferType::StorageBuffer,
						.Size = vertexDataSize
					});

					if (gpuMesh.VertexData != BufferHandle{})
					{
						for (uint32_t bufferOffset = 0; bufferOffset < vertexDataSize; bufferOffset += CopyChunkSize)
						{
							auto commandList = InPool.CreateCommandList();
							commandList.Begin();
							const std::byte* ptr = (const std::byte*)mesh.Vertices.data();
							uint32_t blockSize = Math::Min(bufferOffset + CopyChunkSize, vertexDataSize) - bufferOffset;
							InStagingBuffer.SetData(ptr + bufferOffset, blockSize, 0);
							commandList.CopyToBuffer(gpuMesh.VertexData, bufferOffset, InStagingBuffer, 0, blockSize);
							commandList.End();

							InQueue.SubmitCommandLists({ commandList }, { InFence }, { InFence, fence });

							fence.Wait();
						}
					}
				}

				{
					uint32_t indexDataSize = uint32_t(mesh.Indices.size()) * sizeof(uint32_t);

					gpuMesh.IndexData = Buffer(m_Context, {
						.Type = BufferType::IndexBuffer,
						.Size = indexDataSize
					});

					if (gpuMesh.IndexData.Handle != BufferHandle{})
					{
						for (uint32_t bufferOffset = 0; bufferOffset < indexDataSize; bufferOffset += CopyChunkSize)
						{
							auto commandList = InPool.CreateCommandList();
							commandList.Begin();
							const std::byte* ptr = (const std::byte*)mesh.Indices.data();
							uint32_t blockSize = Math::Min(bufferOffset + CopyChunkSize, indexDataSize) - bufferOffset;
							InStagingBuffer.SetData(ptr + bufferOffset, blockSize, 0);
							commandList.CopyToBuffer(gpuMesh.IndexData, bufferOffset, InStagingBuffer, 0, blockSize);
							commandList.End();

							InQueue.SubmitCommandLists({ commandList }, { InFence }, { InFence, fence });

							fence.Wait();
						}

						gpuMesh.IndexCount = uint32_t(mesh.Indices.size());
					}
				}

				std::atomic_ref<bool>(gpuMesh.IsReady) = true;
			}

			m_Context->Destroy(fence);
		}, &gpuMeshScene.UploadBarrier);
	}

	void WorldRenderer::Reset()
	{
		m_CommandPool.Reset();
	}

	void WorldRenderer::PrepareFrame()
	{
		/*std::scoped_lock lock(m_MeshUploadMutex);

		while (!m_MeshUploadQueue.empty())
		{
			auto meshHandle = m_MeshUploadQueue.back();
			auto& mesh = m_Meshes.Get(meshHandle);

			if (mesh.Textures.empty())
			{
				m_MeshUploadQueue.pop_back();
				continue;
			}

			mesh.TextureOffset = m_TextureCount;
			m_MaterialSet.Write(0, mesh.Textures, m_Sampler, m_TextureCount);
			m_TextureCount += uint32_t(mesh.Textures.size());

			m_MeshUploadQueue.pop_back();
		}*/
	}

	void WorldRenderer::BeginFrame(const Math::Mat4& InViewProjection)
	{
		ExecuteTransfers();
		PrepareFrame();

		m_PushConstants.ViewProjection = InViewProjection;
		m_PushConstants.ObjectVA = m_ObjectStorageBuffer.GetDeviceAddress();
		m_PushConstants.TransformVA = m_TransformStorageBuffer.GetDeviceAddress();

		m_CommandList = m_CommandPool.CreateCommandList();
		m_CommandList.Begin();
		
		m_CommandList.BindPipeline(m_Pipeline);
		m_CommandList.BindDescriptorSet(m_Pipeline, m_MaterialSet);
		m_CommandList.PushConstants(m_Pipeline, &m_PushConstants, sizeof(m_PushConstants));

		m_CommandList.SetViewport({
			.X = 0.0f,
			.Y = 0.0f,
			.Width = float(m_ViewportWidth),
			.Height = float(m_ViewportHeight),
		});

		m_CommandList.SetScissor({
			.X = 0.0f,
			.Y = 0.0f,
			.Width = float(m_ViewportWidth),
			.Height = float(m_ViewportHeight),
		});

		m_CommandList.TransitionImage(m_ColorImage, ImageLayout::Attachment);
		m_CommandList.TransitionImage(m_DepthImage, ImageLayout::Attachment);

		RenderTargetInfo renderInfo = {};
		renderInfo.ColorAttachments.push_back({ .ImageHandle = m_ColorImage });
		renderInfo.DepthAttachment.ImageHandle = m_DepthImage;
		m_CommandList.BeginRendering(renderInfo);
	}

	void WorldRenderer::RenderEntities()
	{
		std::scoped_lock lock(m_RenderMutex);
		uint32_t instanceIndex = 0;
		auto filter = m_World.GetEntityWorld().filter<const Entities::MeshComponent>();
		filter.each([this, &instanceIndex](flecs::entity InEntity, const Entities::MeshComponent& InComponent)
		{
			auto& meshScene = m_GPUMeshScenes.at(InComponent.MeshID);
			auto& mesh = meshScene.Meshes[InComponent.MeshIndex];

			if (!std::atomic_ref<bool>(mesh.IsReady))
				return;

			m_CommandList.BindIndexBuffer(mesh.IndexData, 0);
			m_CommandList.DrawIndexed(mesh.IndexCount, 0, instanceIndex);
			instanceIndex++;
		});
	}

	void WorldRenderer::EndFrame()
	{
		m_CommandList.EndRendering();

		m_CommandList.TransitionImage(m_ColorImage, ImageLayout::ShaderReadOnly);
		m_CommandList.TransitionImage(m_DepthImage, ImageLayout::ShaderReadOnly);

		m_CommandList.End();
		m_GraphicsQueue.SubmitCommandLists({ m_CommandList }, { m_Fence }, { m_Fence });
	}

	void WorldRenderer::SetViewportSize(uint32_t InWidth, uint32_t InHeight)
	{
		m_ViewportWidth = InWidth;
		m_ViewportHeight = InHeight;

		m_ColorImage.Resize(InWidth, InHeight);
		m_DepthImage.Resize(InWidth, InHeight);
	}

	void WorldRenderer::SynchronizeGPUTransform(flecs::entity InEntity)
	{
		ScheduleTransfer([this, InEntity](Queue InQueue, CommandPool InPool, Buffer InStagingBuffer, Fence InFence)
		{
			// FIXME(Peter): Consider replacing this with a lockless solution
			std::scoped_lock lock(m_RenderMutex);
			uint32_t matrixCount = 0;
			uint32_t bufferStart = std::numeric_limits<uint32_t>::max();

			m_World.IterateHierarchy(InEntity, [this, &InStagingBuffer, &matrixCount, &bufferStart](flecs::entity InCurrent)
			{
				const auto* gpuTransform = InCurrent.get<Entities::GPUTransform>();

				if (gpuTransform == nullptr)
					return;

				if (bufferStart == std::numeric_limits<uint32_t>::max())
					bufferStart = gpuTransform->BufferIndex;

				Math::Mat4 parentTransform;
				parentTransform.SetIdentity();

				flecs::entity entity = InCurrent;
				while ((entity = entity.parent()) != flecs::entity::null())
				{
					const auto* translation = entity.get<Entities::Translation>();
					const auto* rotation = entity.get<Entities::Rotation>();
					const auto* scale = entity.get<Entities::Scale>();
					parentTransform = parentTransform * (Math::Mat4::Translation(translation->Value) * Math::Mat4::Rotation(rotation->Value) * Math::Mat4::Scale(scale->Value));
				}

				const auto* translation = InCurrent.get<Entities::Translation>();
				const auto* rotation = InCurrent.get<Entities::Rotation>();
				const auto* scale = InCurrent.get<Entities::Scale>();

				Math::Mat4 transform = parentTransform * (Math::Mat4::Translation(translation->Value) * Math::Mat4::Rotation(rotation->Value) * Math::Mat4::Scale(scale->Value));
				InStagingBuffer.SetData(&transform, sizeof(Math::Mat4), matrixCount * sizeof(Math::Mat4));
				matrixCount++;
			});

			auto commandList = InPool.CreateCommandList();
			commandList.Begin();
			commandList.CopyToBuffer(m_TransformStorageBuffer, bufferStart * sizeof(Math::Mat4), InStagingBuffer, 0, sizeof(Math::Mat4) * matrixCount);
			commandList.End();

			InQueue.SubmitCommandLists({ commandList }, { InFence }, { InFence });
		});
	}

}
