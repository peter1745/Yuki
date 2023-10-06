#include "VulkanAccelerationStructure.hpp"
#include "VulkanRenderDevice.hpp"
#include "Features/VulkanRaytracingFeature.hpp"

namespace Yuki::RHI {

	AccelerationStructureRH VulkanRenderDevice::AccelerationStructureCreate(BufferRH InVertexBuffer, BufferRH InIndexBuffer)
	{
		auto[Handle, AccelerationStructure] = m_AccelerationStructures.Acquire();

		const auto& VertexBuffer = m_Buffers[InVertexBuffer];
		const auto& IndexBuffer = m_Buffers[InIndexBuffer];

		VkAccelerationStructureGeometryTrianglesDataKHR TrianglesData =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
			.pNext = nullptr,
			.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
			.vertexData = { .deviceAddress = VertexBuffer.Address },
			.vertexStride = sizeof(float) * 3,
			.maxVertex = 2,
			.indexType = VK_INDEX_TYPE_UINT32,
			.indexData = { .deviceAddress = IndexBuffer.Address },
			.transformData = {},
		};

		VkAccelerationStructureGeometryKHR Geometry =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
			.pNext = nullptr,
			.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
			.geometry = { .triangles = TrianglesData },
			.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
		};

		VkAccelerationStructureBuildRangeInfoKHR Offset =
		{
			.primitiveCount = 1,
			.primitiveOffset = 0,
			.firstVertex = 0,
			.transformOffset = 0,
		};

		VkAccelerationStructureBuildGeometryInfoKHR BuildInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.pNext = nullptr,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			//.srcAccelerationStructure,
			//.dstAccelerationStructure,
			.geometryCount = 1,
			.pGeometries = &Geometry,
			//.ppGeometries,
			//.scratchData,
		};

		uint32_t NumPrimitives = 1;

		VkAccelerationStructureBuildSizesInfoKHR BuildSizesInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		vkGetAccelerationStructureBuildSizesKHR(m_Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &BuildInfo, &NumPrimitives, &BuildSizesInfo);

		const auto& AccelerationStructureProperties = GetFeature<VulkanRaytracingFeature>().GetAccelerationStructureProperties();

		auto ScratchBuffer = BufferCreate(BuildSizesInfo.buildScratchSize + AccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment, BufferUsage::Storage);

		// TODO(Peter): Compactify

		AccelerationStructure.AccelerationStructureStorage = BufferCreate(BuildSizesInfo.accelerationStructureSize, BufferUsage::AccelerationStructureStorage);

		VkAccelerationStructureCreateInfoKHR AccelerationStructureInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.createFlags = 0,
			.buffer = m_Buffers[AccelerationStructure.AccelerationStructureStorage].Handle,
			.offset = 0,
			.size = BuildSizesInfo.accelerationStructureSize,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			//.deviceAddress,
		};
		vkCreateAccelerationStructureKHR(m_Device, &AccelerationStructureInfo, nullptr, &AccelerationStructure.BottomLevelAS);

		BuildInfo.dstAccelerationStructure = AccelerationStructure.BottomLevelAS;
		BuildInfo.scratchData.deviceAddress = AlignUp(BufferGetDeviceAddress(ScratchBuffer), Cast<uint64_t>(AccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment));

		auto Queue = QueueRequest(QueueType::Graphics);

		auto Fence = FenceCreate();

		auto CmdPool = CommandPoolCreate(Queue);
		auto CmdList = CommandPoolNewList(CmdPool);
		CommandListBegin(CmdList);

		const auto* BuildOffsetsPtr = &Offset;
		vkCmdBuildAccelerationStructuresKHR(m_CommandLists[CmdList].Handle, 1, &BuildInfo, &BuildOffsetsPtr);

		CommandListEnd(CmdList);

		QueueSubmit(Queue, { CmdList }, {}, { Fence });

		FenceWait(Fence);

		BufferDestroy(ScratchBuffer);

		// TLAS

		VkAccelerationStructureDeviceAddressInfoKHR AddressInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = AccelerationStructure.BottomLevelAS
		};

		VkAccelerationStructureInstanceKHR Instance =
		{
			.transform = { .matrix = {
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
			} },
			.instanceCustomIndex = 0,
			.mask = 0xFF,
			.instanceShaderBindingTableRecordOffset = 0,
			.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
			.accelerationStructureReference = vkGetAccelerationStructureDeviceAddressKHR(m_Device, &AddressInfo),
		};

		CmdList = CommandPoolNewList(CmdPool);
		CommandListBegin(CmdList);

		BufferRH InstancesBuffer = BufferCreate(sizeof(VkAccelerationStructureInstanceKHR), BufferUsage::AccelerationStructureBuildInput, true);
		BufferSetData(InstancesBuffer, &Instance, sizeof(Instance));

		VkAccelerationStructureGeometryInstancesDataKHR InstancesData =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.pNext = nullptr,
			.arrayOfPointers = VK_FALSE,
			.data = { .deviceAddress = BufferGetDeviceAddress(InstancesBuffer) },
		};

		VkAccelerationStructureGeometryKHR TopLevelGeometry =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
			.pNext = nullptr,
			.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
			.geometry = { .instances = InstancesData },
			.flags = 0,
		};

		BuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		BuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		BuildInfo.geometryCount = 1;
		BuildInfo.pGeometries = &TopLevelGeometry;
		BuildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

		uint32_t NumInstances = 1;
		vkGetAccelerationStructureBuildSizesKHR(m_Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &BuildInfo, &NumInstances, &BuildSizesInfo);

		AccelerationStructure.TopLevelAccelerationStructureStorage = BufferCreate(BuildSizesInfo.accelerationStructureSize, BufferUsage::AccelerationStructureStorage);

		AccelerationStructureInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.createFlags = 0,
			.buffer = m_Buffers[AccelerationStructure.TopLevelAccelerationStructureStorage].Handle,
			.offset = 0,
			.size = BuildSizesInfo.accelerationStructureSize,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
			//.deviceAddress,
		};
		vkCreateAccelerationStructureKHR(m_Device, &AccelerationStructureInfo, nullptr, &AccelerationStructure.TopLevelAS);

		ScratchBuffer = BufferCreate(BuildSizesInfo.buildScratchSize + AccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment, BufferUsage::Storage);

		BuildInfo.dstAccelerationStructure = AccelerationStructure.TopLevelAS;
		BuildInfo.scratchData.deviceAddress = AlignUp(BufferGetDeviceAddress(ScratchBuffer), Cast<uint64_t>(AccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment));

		VkAccelerationStructureBuildRangeInfoKHR BuildOffsets{ NumInstances, 0, 0, 0 };
		BuildOffsetsPtr = &BuildOffsets;
		vkCmdBuildAccelerationStructuresKHR(m_CommandLists[CmdList].Handle, 1, &BuildInfo, &BuildOffsetsPtr);

		CommandListEnd(CmdList);

		QueueSubmit(Queue, { CmdList }, {}, { Fence });

		FenceWait(Fence);

		BufferDestroy(ScratchBuffer);

		FenceDestroy(Fence);
		CommandPoolDestroy(CmdPool);

		return Handle;
	}

	uint64_t VulkanRenderDevice::AccelerationStructureGetTopLevelAddress(AccelerationStructureRH InAccelerationStructure)
	{
		const auto& AccelerationStructure = m_AccelerationStructures[InAccelerationStructure];

		VkAccelerationStructureDeviceAddressInfoKHR AddressInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = AccelerationStructure.TopLevelAS
		};
		return vkGetAccelerationStructureDeviceAddressKHR(m_Device, &AddressInfo);
	}

	void VulkanRenderDevice::AccelerationStructureDestroy(AccelerationStructureRH InAccelerationStructure)
	{
	}

}
