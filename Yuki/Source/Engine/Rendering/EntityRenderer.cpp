#include "Rendering/EntityRenderer.hpp"
#include "Rendering/RenderContext.hpp"
#include "Rendering/DescriptorSetBuilder.hpp"
#include "Entities/TransformComponents.hpp"

namespace Yuki {

	WorldRenderer::WorldRenderer(RenderContext* InContext, World& InWorld)
		: m_Context(InContext), m_World(InWorld)
	{
		m_GraphicsQueue = { m_Context->GetGraphicsQueue(), m_Context };

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

	void WorldRenderer::CreateGPUObject(flecs::entity InEntity)
	{
		/*const auto* meshComponent = InEntity.get<Entities::MeshComponent>();
		const auto& mesh = m_Meshes.Get(meshComponent->Value);

		m_EntityInstanceMap[InEntity] = m_LastInstanceID;

		for (size_t i = 0; i < mesh.Instances.size(); i++)
		{
			const auto& meshInstance = mesh.Instances[i];

			auto& cpuInstance = m_CPUObjects.emplace_back();
			cpuInstance.Mesh = meshComponent->Value;
			cpuInstance.InstanceIndex = i;

			GPUObject gpuObject =
			{
				.VertexVA = mesh.Sources[meshInstance.SourceIndex].VertexData.GetDeviceAddress(),
				.MaterialVA = mesh.MaterialStorageBuffer.GetDeviceAddress(),
				.BaseTextureOffset = mesh.TextureOffset
			};

			m_ObjectStagingBuffer.SetData(&gpuObject, sizeof(GPUObject), m_LastInstanceID * sizeof(GPUObject));
			m_TransformStagingBuffer.SetData(&meshInstance.Transform, sizeof(Math::Mat4), m_LastInstanceID * sizeof(Math::Mat4));
			m_LastInstanceID++;
		}

		m_Context->GetTransferScheduler().Schedule([this, baseIndex = m_EntityInstanceMap[InEntity], &mesh](CommandListHandle InCommandList)
		{
			CommandList commandList{InCommandList, m_Context};
			commandList.CopyToBuffer(m_ObjectStorageBuffer, baseIndex * sizeof(GPUObject), m_ObjectStagingBuffer, baseIndex * sizeof(GPUObject), uint32_t(mesh.Instances.size()) * sizeof(GPUObject));
			commandList.CopyToBuffer(m_TransformStorageBuffer, baseIndex * sizeof(Math::Mat4), m_TransformStagingBuffer, baseIndex * sizeof(Math::Mat4), uint32_t(mesh.Instances.size()) * sizeof(Math::Mat4));
		}, [this, handle = meshComponent->Value]()
		{
			m_Meshes.MarkReady(handle);
		}, {}, {});*/
	}

	/*MeshHandle WorldRenderer::SubmitForUpload(Mesh InMesh)
	{
		auto[handle, mesh] = m_Meshes.Insert(InMesh);
		
		std::scoped_lock lock(m_MeshUploadMutex);
		m_MeshUploadQueue.push_back(handle);
		YUKI_UNUSED(mesh);

		return handle;
	}*/

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
		/*uint32_t instanceIndex = 0;

		for (const auto& instance : m_CPUObjects)
		{
			if (!m_Meshes.IsValid(instance.Mesh))
				return;

			const auto& mesh = m_Meshes.Get(instance.Mesh);
			const auto& meshInstance = mesh.Instances[instance.InstanceIndex];
			const auto& meshData = mesh.Sources[meshInstance.SourceIndex];

			m_CommandList.BindIndexBuffer(meshData.IndexBuffer, 0);
			m_CommandList.DrawIndexed(meshData.IndexCount, 0, instanceIndex);
			instanceIndex++;
		}*/
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
		/*if (!m_EntityInstanceMap.contains(InEntity))
			return;

		uint32_t baseTransformIndex = m_EntityInstanceMap[InEntity];

		const auto* meshComp = InEntity.get<Entities::MeshComponent>();
		const auto& mesh = m_Meshes.Get(meshComp->Value);
		
		const auto& translation = InEntity.get<Entities::Translation>()->Value;
		const auto& rotation = InEntity.get<Entities::Rotation>()->Value;
		const auto& scale = InEntity.get<Entities::Scale>()->Value;

		Math::Mat4 entityTransform = Math::Mat4::Translation(translation) * Math::Mat4::Rotation(rotation) * Math::Mat4::Scale({ scale, scale, scale });
		for (uint32_t i = 0; i < uint32_t(mesh.Instances.size()); i++)
		{
			const auto& meshInstance = mesh.Instances[i];
			Math::Mat4 transform = entityTransform * meshInstance.Transform;
			m_TransformStagingBuffer.SetData(&transform, sizeof(Math::Mat4), (baseTransformIndex + i) * sizeof(Math::Mat4));
		}

		m_Context->GetTransferScheduler().Schedule([context = m_Context, storageBuffer = m_TransformStorageBuffer, stagingBuffer = m_TransformStagingBuffer, baseIndex = baseTransformIndex, &mesh](CommandListHandle InCommandList)
		{
			CommandList commandList{InCommandList, context};
			commandList.CopyToBuffer(storageBuffer, baseIndex * sizeof(Math::Mat4), stagingBuffer, baseIndex * sizeof(Math::Mat4), uint32_t(mesh.Instances.size()) * sizeof(Math::Mat4));
		}, [](){}, {}, {});*/
	}

}
