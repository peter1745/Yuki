#include "RTRenderer.hpp"
#include "Engine/Common/Timer.hpp"

namespace Yuki {

	RTRenderer::RTRenderer(RHI::Context context)
		: m_Context(context)
	{
		m_TransferManager = Unique<TransferManager>::New(m_Context);

		m_GraphicsQueue = m_Context.RequestQueue(RHI::QueueType::Graphics);

		m_PipelineLayout = RHI::PipelineLayout::Create(m_Context, {
			.PushConstantSize = sizeof(PushConstants)
		});

		 m_Pipeline = RHI::RayTracingPipeline::Create(m_Context, {
			.Layout = m_PipelineLayout,
			.RayGenShader = { "Shaders/RayGen.glsl", RHI::ShaderStage::RayGeneration },
			.HitShaderGroups = {
				{
					{ "Shaders/RayClosestHit.glsl", RHI::ShaderStage::RayClosestHit },
				},
				{
					{ "Shaders/RayClosestHit.glsl", RHI::ShaderStage::RayClosestHit },
					{ "Shaders/RayAnyHit.glsl", RHI::ShaderStage::RayAnyHit }
				},
			},
			.MissShader = { "Shaders/RayMiss.glsl", RHI::ShaderStage::RayMiss }
		});

		m_PushConstants.OutputImageHandle = m_HeapIndices.Acquire();

		m_DescriptorHeap = RHI::DescriptorHeap::Create(m_Context, 65536);

		m_CommandPool = RHI::CommandPool::Create(m_Context, m_GraphicsQueue);

		m_MaterialsBuffer = m_TransferManager->CreateBuffer(65536 * sizeof(MeshMaterial), RHI::BufferUsage::Storage);
		m_PushConstants.Materials = m_MaterialsBuffer.GetDeviceAddress();

		m_GPUMeshBuffer = m_TransferManager->CreateBuffer(65536 * sizeof(GPUMeshData), RHI::BufferUsage::Storage);
		m_PushConstants.Geometries = m_GPUMeshBuffer.GetDeviceAddress();

		m_HitShaderHandles = m_TransferManager->CreateBuffer(65536, RHI::BufferUsage::ShaderBindingTable, RHI::BufferFlags::Mapped);

		m_BuildFence = RHI::Fence::Create(m_Context);
		m_Builder = RHI::AccelerationStructureBuilder::Create(m_Context, m_TransferManager.Get());

		// TEMP
		auto sampler = RHI::Sampler::Create(m_Context);
		m_PushConstants.DefaultSamplerHandle = m_HeapIndices.Acquire();
		m_DescriptorHeap.WriteSamplers(m_PushConstants.DefaultSamplerHandle, { sampler });

		m_PushConstants.CameraZOffset = 1.0f / glm::tan(0.5f * glm::radians(90.0f));
	}

	void RTRenderer::Render(RHI::Image target, RHI::Fence fence, const CameraData& cameraData)
	{
		m_TransferManager->Execute({ m_BuildFence });

		if (m_RebuildAccelerationStructure && m_BuildFence.GetValue() == m_BuildFence.GetCurrentValue())
		{
			m_AccelerationStructure = m_Builder.Build();
			m_PushConstants.TopLevelAS = m_AccelerationStructure.GetTopLevelAddress();
			m_RebuildAccelerationStructure = false;
		}

		m_PushConstants.ViewPos = cameraData.Position;
		m_PushConstants.CameraX = cameraData.Rotation * Vec3{ 1.0f, 0.0f, 0.0f };
		m_PushConstants.CameraY = cameraData.Rotation * Vec3{ 0.0f, 1.0f, 0.0f };

		m_CommandPool.Reset();

		auto cmd = m_CommandPool.NewList();
		cmd.Begin();
		cmd.ImageBarrier({ { target }, { RHI::ImageLayout::General } });

		m_DescriptorHeap.WriteStorageImages(m_PushConstants.OutputImageHandle, { target.GetDefaultView() });

		cmd.PushConstants(m_PipelineLayout, RHI::ShaderStage::RayGeneration, &m_PushConstants, sizeof(PushConstants));
		cmd.BindDescriptorHeap(m_PipelineLayout, RHI::PipelineBindPoint::RayTracing, m_DescriptorHeap);
		cmd.BindPipeline(m_Pipeline);
		cmd.TraceRays(m_Pipeline, target.GetWidth(), target.GetHeight(), m_HitShaderHandles.GetDeviceAddress(), 2);
		cmd.ImageBarrier({ { target }, { RHI::ImageLayout::Present } });
		cmd.End();

		m_GraphicsQueue.Submit({ cmd }, { fence }, { fence });
	}

	void RTRenderer::AddMesh(const Model& model)
	{
		// Intentional copy
		auto materials = model.Materials;

		Timer::Start();
		for (auto& material : materials)
		{
			uint32_t handle = m_HeapIndices.Acquire();

			if (material.BaseColorTextureIndex == -1)
				continue;

			const auto& texture = model.Textures[material.BaseColorTextureIndex];

			material.BaseColorTextureIndex = handle;

			auto image = m_TransferManager->CreateImage(texture.Data, {
				.Width = texture.Width,
				.Height = texture.Height,
				.Format = RHI::ImageFormat::RGBA8,
				.Usage = RHI::ImageUsage::Sampled
			});

			m_DescriptorHeap.WriteSampledImages(handle, { image.GetDefaultView() });
		}
		Timer::Stop("Texture Upload");

		m_TransferManager->UploadBufferData<MeshMaterial>(m_MaterialsBuffer, materials, m_NumMaterials);
		m_NumMaterials += Cast<uint32_t>(materials.size());

		// TODO(Peter): Determine a good method for sorting meshes into BLASs
		DynamicArray<RHI::BlasID> blases;
		DynamicArray<RHI::GeometryID> geometries;
		for (uint32_t i = 0; i < model.Meshes.size(); i++)
		{
			const auto& mesh = model.Meshes[i];

			auto blas = m_Builder.CreateBLAS();
			RHI::Buffer indexBuffer = m_TransferManager->CreateBuffer<uint32_t>(mesh.Indices, RHI::BufferUsage::AccelerationStructureBuildInput);
			RHI::Buffer shadingAttribsBuffer = m_TransferManager->CreateBuffer<ShadingAttributes>(mesh.ShadingAttributes, RHI::BufferUsage::Storage);

			m_TransferManager->UploadBufferData<GPUMeshData>(m_GPUMeshBuffer, {
				{
					.IndexBufferAddress = indexBuffer.GetDeviceAddress(),
					.ShadingAttributesAddress = shadingAttribsBuffer.GetDeviceAddress(),
					.MaterialIndex = mesh.MaterialIndex
				}
			}, Cast<uint32_t>(blases.size()));

			bool isOpaque = materials[mesh.MaterialIndex].AlphaBlending == 0;
			auto geometryID = m_Builder.AddGeometry(blas, mesh.Positions, indexBuffer, Cast<uint32_t>(mesh.Indices.size()), isOpaque);
			blases.push_back(blas);
			geometries.push_back(geometryID);

			uint32_t sbtIndex = isOpaque ? 0 : 1;
			m_Pipeline.WriteHandle(m_HitShaderHandles.GetMappedMemory(), i, sbtIndex);
		}

		for (const auto& scene : model.Scenes)
		{
			for (const auto& instance : scene.Instances)
			{
				m_Builder.AddInstance(blases[instance.MeshIndex], geometries[instance.MeshIndex], instance.Transform, instance.MeshIndex, instance.MeshIndex);
			}
		}

		m_RebuildAccelerationStructure = true;
	}

}
