#include "RTRenderer.hpp"
#include "Engine/Common/Timer.hpp"

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

		//m_AccelerationStructure = RHI::AccelerationStructure::Create(m_Context);
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
#if 0
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

			RHI::Image image = RHI::Image::Create(m_Context, texture.Width, texture.Height,
				RHI::ImageFormat::RGBA8,
				RHI::ImageUsage::Sampled |
				RHI::ImageUsage::TransferDest |
				RHI::ImageUsage::HostTransfer);
			image.SetData(texture.Data.data());
			image.Transition(RHI::ImageLayout::ShaderReadOnly);

			m_DescriptorHeap.WriteSampledImages(handle, { image.GetDefaultView() });
		}
		Timer::Stop("Texture Upload");

		m_MaterialsBuffer.Set<MeshMaterial>(materials, m_NumMaterials);
		m_NumMaterials += Cast<uint32_t>(materials.size());

		DynamicArray<RHI::GeometryID> geometries;
		geometries.resize(model.Meshes.size());

		uint32_t geometryIndex = 0;

		Timer::Start();
		for (uint32_t i = 0; i < model.Meshes.size(); i++)
		{
			const auto& mesh = model.Meshes[i];

			/*
			
			RHI::Buffer indexBuffer = m_TransferManager.CreateBuffer(mesh.Indices, RHI::BufferUsage::AccelerationStructureBuildInput, { m_AccelerationStructureBuildFence });
			RHI::Buffer shadingAttribsBuffer = m_TransferManager.CreateBuffer(mesh.ShadingAttributes, RHI::BufferUsage::Storage, { m_AccelerationStructureBuildFence });

			*/

			RHI::Buffer indexBuffer = RHI::Buffer::Create(m_Context, mesh.Indices.size() * sizeof(uint32_t),
				RHI::BufferUsage::AccelerationStructureBuildInput,
				RHI::BufferFlags::Mapped |
				RHI::BufferFlags::DeviceLocal);
			indexBuffer.Set<uint32_t>(mesh.Indices);

			RHI::Buffer shadingAttribsBuffer = RHI::Buffer::Create(m_Context, mesh.ShadingAttributes.size() * sizeof(ShadingAttributes),
				RHI::BufferUsage::Storage |
				RHI::BufferUsage::TransferDst,
				RHI::BufferFlags::Mapped |
				RHI::BufferFlags::DeviceLocal);
			shadingAttribsBuffer.Set<ShadingAttributes>(mesh.ShadingAttributes);

			/*
			m_TransferManager.UploadBufferData<GPUMeshData>(m_GPUMeshBuffer, {
				.IndexBufferAddress = indexBuffer.GetDeviceAddress(),
				.ShadingAttributesAddress = shadingAttribsBuffer.GetDeviceAddress(),
				.MaterialIndex = mesh.MaterialIndex
			}, { m_MeshBufferFence });
			*/

			m_GPUMeshBuffer.Set<GPUMeshData>({ {
				.IndexBufferAddress = indexBuffer.GetDeviceAddress(),
				.ShadingAttributesAddress = shadingAttribsBuffer.GetDeviceAddress(),
				.MaterialIndex = mesh.MaterialIndex
			} }, geometryIndex);

			geometries[i] = m_AccelerationStructure.AddGeometry(mesh.Positions, indexBuffer, Cast<uint32_t>(mesh.Indices.size()));

			uint32_t sbtIndex = 0;
			if (materials[mesh.MaterialIndex].AlphaBlending)
			{
				sbtIndex = 1;
			}

			m_Pipeline.WriteHandle(m_HitShaderHandles.GetMappedMemory(), i, sbtIndex);
			geometryIndex++;
		}
		Timer::Stop("BLAS Building");

		Timer::Start();
		for (const auto& scene : model.Scenes)
		{
			for (const auto& instance : scene.Instances)
			{
				m_AccelerationStructure.AddInstance(geometries[instance.MeshIndex], instance.Transform, instance.MeshIndex, instance.MeshIndex);
			}
		}
		Timer::Stop("TLAS Building");

		m_PushConstants.TopLevelAS = m_AccelerationStructure.GetTopLevelAddress();
#else
		auto builder = RHI::AccelerationStructureBuilder::Create(m_Context);

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

			RHI::Image image = RHI::Image::Create(m_Context, texture.Width, texture.Height,
				RHI::ImageFormat::RGBA8,
				RHI::ImageUsage::Sampled |
				RHI::ImageUsage::TransferDest |
				RHI::ImageUsage::HostTransfer);
			image.SetData(texture.Data.data());
			image.Transition(RHI::ImageLayout::ShaderReadOnly);

			m_DescriptorHeap.WriteSampledImages(handle, { image.GetDefaultView() });
		}
		Timer::Stop("Texture Upload");

		m_MaterialsBuffer.Set<MeshMaterial>(materials, m_NumMaterials);
		m_NumMaterials += Cast<uint32_t>(materials.size());

		// TODO(Peter): Determine a good method for sorting meshes into BLASs
		DynamicArray<RHI::BlasID> blases;
		DynamicArray<RHI::GeometryID> geometries;
		for (uint32_t i = 0; i < model.Meshes.size(); i++)
		{
			const auto& mesh = model.Meshes[i];

			auto blas = builder.CreateBLAS();
			RHI::Buffer indexBuffer = RHI::Buffer::Create(m_Context, mesh.Indices.size() * sizeof(uint32_t),
				RHI::BufferUsage::AccelerationStructureBuildInput,
				RHI::BufferFlags::Mapped |
				RHI::BufferFlags::DeviceLocal);
			indexBuffer.Set<uint32_t>(mesh.Indices);

			RHI::Buffer shadingAttribsBuffer = RHI::Buffer::Create(m_Context, mesh.ShadingAttributes.size() * sizeof(ShadingAttributes),
				RHI::BufferUsage::Storage |
				RHI::BufferUsage::TransferDst,
				RHI::BufferFlags::Mapped |
				RHI::BufferFlags::DeviceLocal);
			shadingAttribsBuffer.Set<ShadingAttributes>(mesh.ShadingAttributes);

			m_GPUMeshBuffer.Set<GPUMeshData>({ {
				.IndexBufferAddress = indexBuffer.GetDeviceAddress(),
				.ShadingAttributesAddress = shadingAttribsBuffer.GetDeviceAddress(),
				.MaterialIndex = mesh.MaterialIndex
			} }, blases.size());

			auto geometryID = builder.AddGeometry(blas, mesh.Positions, indexBuffer, Cast<uint32_t>(mesh.Indices.size()), materials[mesh.MaterialIndex].AlphaBlending == 1);
			blases.push_back(blas);
			geometries.push_back(geometryID);

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
				const auto& mesh = model.Meshes[instance.MeshIndex];
				builder.AddInstance(blases[instance.MeshIndex], geometries[instance.MeshIndex], instance.Transform, instance.MeshIndex, instance.MeshIndex);
			}
		}

		m_AccelerationStructure = builder.Build();
		m_PushConstants.TopLevelAS = m_AccelerationStructure.GetTopLevelAddress();
#endif
	}

}
