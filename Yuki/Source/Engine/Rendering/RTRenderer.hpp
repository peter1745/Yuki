#pragma once

#include "Engine/Containers/IndexFreeList.hpp"
#include "Engine/RHI/RenderHandles.hpp"

#include "Mesh.hpp"

namespace Yuki {

	using MeshID = UniqueID;
	using InstanceID = UniqueID;

	class RTRenderer
	{
	public:
		struct CameraData
		{
			Vec3 Position;
			Quat Rotation;
		};

	public:
		RTRenderer(RHI::Context context);

		void Render(RHI::Image target, RHI::Fence fence, const CameraData& cameraData);

	public:
		void AddMesh(const Model& model);

	private:
		RHI::Context m_Context;
		RHI::Queue m_GraphicsQueue;
		RHI::PipelineLayout m_PipelineLayout;
		RHI::RayTracingPipeline m_Pipeline;
		RHI::AccelerationStructure m_AccelerationStructure;
		RHI::DescriptorHeap m_DescriptorHeap;

		RHI::CommandPool m_CommandPool;

		IndexFreeList m_HeapIndices;

		RHI::Buffer m_MaterialsBuffer;
		uint32_t m_NumMaterials = 0;

		struct GPUMeshData
		{
			uint64_t IndexBufferAddress;
			uint64_t ShadingAttributesAddress;
			uint32_t MaterialIndex;
		};
		DynamicArray<GPUMeshData> m_GPUMeshes;
		RHI::Buffer m_GPUMeshBuffer;

		RHI::Buffer m_HitShaderHandles;

		struct PushConstants
		{
			uint64_t TopLevelAS = 0;
			Yuki::Vec3 ViewPos = { 0.0f, 0.0f, 10.0f };
			Yuki::Vec3 CameraX = { 1.0f, 0.0f, 0.0f };
			Yuki::Vec3 CameraY = { 0.0f, 1.0f, 0.0f };
			float CameraZOffset = 0.0f;
			uint64_t Geometries = 0;
			uint64_t Materials = 0;
			uint32_t OutputImageHandle = 0;
			uint32_t DefaultSamplerHandle = 0;
		} m_PushConstants;
	};

}
