#include "RTRenderer.hpp"

namespace Yuki {

	RTRenderer::RTRenderer(RHI::Context context)
		: m_Context(context)
	{
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

		m_AccelerationStructure = RHI::AccelerationStructure::Create(m_Context);
		m_PushConstants.OutputImageHandle = m_HeapIndices.Acquire();

		m_DescriptorHeap = RHI::DescriptorHeap::Create(m_Context, 65536);

		m_CommandPool = RHI::CommandPool::Create(m_Context, m_GraphicsQueue);

		m_MaterialsBuffer = RHI::Buffer::Create(m_Context, 65536 * sizeof(MeshMaterial),
			RHI::BufferUsage::Storage |
			RHI::BufferUsage::TransferDst,
			RHI::BufferFlags::Mapped |
			RHI::BufferFlags::DeviceLocal);
		m_PushConstants.Materials = m_MaterialsBuffer.GetDeviceAddress();

		m_GPUMeshBuffer = RHI::Buffer::Create(m_Context, 65536 * sizeof(GPUMeshData),
			RHI::BufferUsage::Storage |
			RHI::BufferUsage::TransferDst,
			RHI::BufferFlags::Mapped |
			RHI::BufferFlags::DeviceLocal);
		m_PushConstants.Geometries = m_GPUMeshBuffer.GetDeviceAddress();

		m_HitShaderHandles = RHI::Buffer::Create(m_Context, 65536,
			RHI::BufferUsage::ShaderBindingTable,
			RHI::BufferFlags::Mapped |
			RHI::BufferFlags::DeviceLocal);

		// TEMP
		auto sampler = RHI::Sampler::Create(m_Context);
		m_PushConstants.DefaultSamplerHandle = m_HeapIndices.Acquire();
		m_DescriptorHeap.WriteSamplers(m_PushConstants.DefaultSamplerHandle, { sampler });

		m_PushConstants.CameraZOffset = 1.0f / glm::tan(0.5f * glm::radians(90.0f));
	}

	void RTRenderer::Render(RHI::Image target, RHI::Fence fence, const CameraData& cameraData)
	{
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

		for (auto& material : materials)
		{
			uint32_t handle = m_HeapIndices.Acquire();

			if (material.BaseColorTextureIndex == -1)
				continue;

			const auto& texture = model.Textures[material.BaseColorTextureIndex];

			material.BaseColorTextureIndex = handle;

			RHI::Image image = RHI::Image::Create(m_Context, texture.Width, texture.Height,
				RHI::ImageFormat::RGBA8,
				RHI::ImageUsage::Sampled |
				RHI::ImageUsage::TransferDest |
				RHI::ImageUsage::HostTransfer);
			image.SetData(texture.Data.data());
			image.Transition(RHI::ImageLayout::ShaderReadOnly);

			m_DescriptorHeap.WriteSampledImages(handle, { image.GetDefaultView() });
		}

		m_MaterialsBuffer.Set<MeshMaterial>(materials, m_NumMaterials);
		m_NumMaterials += Cast<uint32_t>(materials.size());

		DynamicArray<RHI::GeometryID> geometries;

		geometries.resize(model.Meshes.size());

		for (uint32_t i = 0; i < model.Meshes.size(); i++)
		{
			const auto& mesh = model.Meshes[i];

			RHI::Buffer indexBuffer = RHI::Buffer::Create(m_Context, mesh.Indices.size() * sizeof(uint32_t),
				RHI::BufferUsage::Storage |
				RHI::BufferUsage::TransferDst,
				RHI::BufferFlags::Mapped |
				RHI::BufferFlags::DeviceLocal);
			indexBuffer.Set<uint32_t>(mesh.Indices);

			RHI::Buffer shadingAttribsBuffer = RHI::Buffer::Create(m_Context, mesh.ShadingAttributes.size() * sizeof(ShadingAttributes),
				RHI::BufferUsage::Storage |
				RHI::BufferUsage::TransferDst,
				RHI::BufferFlags::Mapped |
				RHI::BufferFlags::DeviceLocal);
			shadingAttribsBuffer.Set<ShadingAttributes>(mesh.ShadingAttributes);

			auto& gpuMesh = m_GPUMeshes.emplace_back(GPUMeshData{
				.IndexBufferAddress = indexBuffer.GetDeviceAddress(),
				.ShadingAttributesAddress = shadingAttribsBuffer.GetDeviceAddress(),
				.MaterialIndex = mesh.MaterialIndex
			});

			m_GPUMeshBuffer.Set<GPUMeshData>({ gpuMesh }, Cast<uint32_t>(m_GPUMeshes.size()) - 1);

			geometries[i] = m_AccelerationStructure.AddGeometry(mesh.Positions, mesh.Indices);

			uint32_t sbtIndex = 0;
			if (materials[mesh.MaterialIndex].AlphaBlending)
			{
				sbtIndex = 1;
			}

			m_Pipeline.WriteHandle(m_HitShaderHandles.GetMappedMemory(), i, sbtIndex);
		}

		for (const auto& scene : model.Scenes)
		{
			for (const auto& instance : scene.Instances)
			{
				m_AccelerationStructure.AddInstance(geometries[instance.MeshIndex], instance.Transform, instance.MeshIndex, instance.MeshIndex);
			}
		}

		m_PushConstants.TopLevelAS = m_AccelerationStructure.GetTopLevelAddress();
	}

}
