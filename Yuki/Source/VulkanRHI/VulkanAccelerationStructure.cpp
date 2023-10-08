#include "VulkanRHI.hpp"

#include "Features/VulkanRaytracingFeature.hpp"

namespace Yuki::RHI {

	AccelerationStructure AccelerationStructure::Create(Context InContext, BufferRH InVertexBuffer, BufferRH InIndexBuffer)
	{
		auto AccelerationStructure = new Impl();
		AccelerationStructure->Ctx = InContext;

		VkAccelerationStructureGeometryTrianglesDataKHR TrianglesData =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
			.pNext = nullptr,
			.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
			.vertexData = { .deviceAddress = InVertexBuffer->Address },
			.vertexStride = sizeof(float) * 3,
			.maxVertex = 2,
			.indexType = VK_INDEX_TYPE_UINT32,
			.indexData = { .deviceAddress = InIndexBuffer->Address },
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
		vkGetAccelerationStructureBuildSizesKHR(InContext->Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &BuildInfo, &NumPrimitives, &BuildSizesInfo);

		const auto& AccelerationStructureProperties = InContext->GetFeature<VulkanRaytracingFeature>().GetAccelerationStructureProperties();

		auto ScratchBuffer = Buffer::Create(InContext, BuildSizesInfo.buildScratchSize + AccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment, BufferUsage::Storage);

		// TODO(Peter): Compactify

		AccelerationStructure->AccelerationStructureStorage = Buffer::Create(InContext, BuildSizesInfo.accelerationStructureSize, BufferUsage::AccelerationStructureStorage);

		VkAccelerationStructureCreateInfoKHR AccelerationStructureInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.createFlags = 0,
			.buffer = AccelerationStructure->AccelerationStructureStorage->Handle,
			.offset = 0,
			.size = BuildSizesInfo.accelerationStructureSize,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		};
		vkCreateAccelerationStructureKHR(InContext->Device, &AccelerationStructureInfo, nullptr, &AccelerationStructure->BottomLevelAS);

		BuildInfo.dstAccelerationStructure = AccelerationStructure->BottomLevelAS;
		BuildInfo.scratchData.deviceAddress = AlignUp(ScratchBuffer->Address, Cast<uint64_t>(AccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment));

		auto Queue = InContext.RequestQueue(QueueType::Graphics);

		auto Fence = Fence::Create(InContext);

		auto CmdPool = CommandPool::Create(InContext, Queue);
		auto CmdList = CmdPool.NewList();
		CmdList.Begin();

		const auto* BuildOffsetsPtr = &Offset;
		vkCmdBuildAccelerationStructuresKHR(CmdList->Handle, 1, &BuildInfo, &BuildOffsetsPtr);

		CmdList.End();

		Queue.Submit({ CmdList }, {}, { Fence });

		Fence.Wait();

		ScratchBuffer.Destroy();

		// TLAS

		VkAccelerationStructureDeviceAddressInfoKHR AddressInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = AccelerationStructure->BottomLevelAS
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
			.accelerationStructureReference = vkGetAccelerationStructureDeviceAddressKHR(InContext->Device, &AddressInfo),
		};

		CmdList = CmdPool.NewList();
		CmdList.Begin();

		auto InstancesBuffer = Buffer::Create(InContext, sizeof(VkAccelerationStructureInstanceKHR), BufferUsage::AccelerationStructureBuildInput, true);
		InstancesBuffer.SetData(&Instance, sizeof(Instance));

		VkAccelerationStructureGeometryInstancesDataKHR InstancesData =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.pNext = nullptr,
			.arrayOfPointers = VK_FALSE,
			.data = { .deviceAddress = InstancesBuffer->Address },
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
		vkGetAccelerationStructureBuildSizesKHR(InContext->Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &BuildInfo, &NumInstances, &BuildSizesInfo);

		AccelerationStructure->TopLevelAccelerationStructureStorage = Buffer::Create(InContext, BuildSizesInfo.accelerationStructureSize, BufferUsage::AccelerationStructureStorage);

		AccelerationStructureInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.createFlags = 0,
			.buffer = AccelerationStructure->TopLevelAccelerationStructureStorage->Handle,
			.offset = 0,
			.size = BuildSizesInfo.accelerationStructureSize,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		};
		vkCreateAccelerationStructureKHR(InContext->Device, &AccelerationStructureInfo, nullptr, &AccelerationStructure->TopLevelAS);

		ScratchBuffer = Buffer::Create(InContext, BuildSizesInfo.buildScratchSize + AccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment, BufferUsage::Storage);

		BuildInfo.dstAccelerationStructure = AccelerationStructure->TopLevelAS;
		BuildInfo.scratchData.deviceAddress = AlignUp(ScratchBuffer->Address, Cast<uint64_t>(AccelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment));

		VkAccelerationStructureBuildRangeInfoKHR BuildOffsets{ NumInstances, 0, 0, 0 };
		BuildOffsetsPtr = &BuildOffsets;
		vkCmdBuildAccelerationStructuresKHR(CmdList->Handle, 1, &BuildInfo, &BuildOffsetsPtr);

		CmdList.End();

		Queue.Submit({ CmdList }, {}, { Fence });

		Fence.Wait();

		ScratchBuffer.Destroy();

		Fence.Destroy();
		CmdPool.Destroy();

		return { AccelerationStructure };
	}

	uint64_t AccelerationStructure::GetTopLevelAddress()
	{
		VkAccelerationStructureDeviceAddressInfoKHR AddressInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = m_Impl->TopLevelAS
		};
		return vkGetAccelerationStructureDeviceAddressKHR(m_Impl->Ctx->Device, &AddressInfo);
	}

	/*void VulkanRenderDevice::AccelerationStructureDestroy(AccelerationStructureRH InAccelerationStructure)
	{
	}*/

}
