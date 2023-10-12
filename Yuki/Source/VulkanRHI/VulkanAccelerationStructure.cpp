#include "VulkanRHI.hpp"

#include "Features/VulkanRaytracingFeature.hpp"

namespace Yuki::RHI {

	AccelerationStructure AccelerationStructure::Create(Context context, BufferRH vertexBuffer, BufferRH indexBuffer)
	{
		auto accelerationStructure = new Impl();
		accelerationStructure->Ctx = context;

		VkAccelerationStructureGeometryTrianglesDataKHR trianglesData =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
			.pNext = nullptr,
			.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
			.vertexData = { .deviceAddress = vertexBuffer->Address },
			.vertexStride = sizeof(float) * 3,
			.maxVertex = 3,
			.indexType = VK_INDEX_TYPE_UINT32,
			.indexData = { .deviceAddress = indexBuffer->Address },
			.transformData = {},
		};

		VkAccelerationStructureGeometryKHR geometry =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
			.pNext = nullptr,
			.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
			.geometry = { .triangles = trianglesData },
			.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
		};

		VkAccelerationStructureBuildRangeInfoKHR offset =
		{
			.primitiveCount = 2,
			.primitiveOffset = 0,
			.firstVertex = 0,
			.transformOffset = 0,
		};

		VkAccelerationStructureBuildGeometryInfoKHR buildInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.pNext = nullptr,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.geometryCount = 1,
			.pGeometries = &geometry,
		};

		uint32_t numPrimitives = 2;

		VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		vkGetAccelerationStructureBuildSizesKHR(context->Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &numPrimitives, &buildSizesInfo);

		const auto& accelerationStructureProperties = context->GetFeature<VulkanRaytracingFeature>().GetAccelerationStructureProperties();

		auto scratchBuffer = Buffer::Create(context, buildSizesInfo.buildScratchSize + accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment, BufferUsage::Storage);

		// TODO(Peter): Compactify

		accelerationStructure->AccelerationStructureStorage = Buffer::Create(context, buildSizesInfo.accelerationStructureSize, BufferUsage::AccelerationStructureStorage);

		VkAccelerationStructureCreateInfoKHR accelerationStructureInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.createFlags = 0,
			.buffer = accelerationStructure->AccelerationStructureStorage->Handle,
			.offset = 0,
			.size = buildSizesInfo.accelerationStructureSize,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		};
		vkCreateAccelerationStructureKHR(context->Device, &accelerationStructureInfo, nullptr, &accelerationStructure->BottomLevelAS);

		buildInfo.dstAccelerationStructure = accelerationStructure->BottomLevelAS;
		buildInfo.scratchData.deviceAddress = AlignUp(scratchBuffer->Address, Cast<uint64_t>(accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment));

		auto queue = context.RequestQueue(QueueType::Graphics);

		auto fence = Fence::Create(context);

		auto cmdPool = CommandPool::Create(context, queue);
		auto cmdList = cmdPool.NewList();
		cmdList.Begin();

		const auto* buildOffsetsPtr = &offset;
		vkCmdBuildAccelerationStructuresKHR(cmdList->Handle, 1, &buildInfo, &buildOffsetsPtr);

		cmdList.End();

		queue.Submit({ cmdList }, {}, { fence });

		fence.Wait();

		scratchBuffer.Destroy();

		// TLAS

		VkAccelerationStructureDeviceAddressInfoKHR addressInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = accelerationStructure->BottomLevelAS
		};

		VkAccelerationStructureInstanceKHR instance =
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
			.accelerationStructureReference = vkGetAccelerationStructureDeviceAddressKHR(context->Device, &addressInfo),
		};

		cmdList = cmdPool.NewList();
		cmdList.Begin();

		auto instancesBuffer = Buffer::Create(context, sizeof(VkAccelerationStructureInstanceKHR), BufferUsage::AccelerationStructureBuildInput, true);
		instancesBuffer.SetData(&instance, sizeof(instance));

		VkAccelerationStructureGeometryInstancesDataKHR instancesData =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.pNext = nullptr,
			.arrayOfPointers = VK_FALSE,
			.data = { .deviceAddress = instancesBuffer->Address },
		};

		VkAccelerationStructureGeometryKHR topLevelGeometry =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
			.pNext = nullptr,
			.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
			.geometry = { .instances = instancesData },
			.flags = 0,
		};

		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &topLevelGeometry;
		buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

		uint32_t numInstances = 1;
		vkGetAccelerationStructureBuildSizesKHR(context->Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &numInstances, &buildSizesInfo);

		accelerationStructure->TopLevelAccelerationStructureStorage = Buffer::Create(context, buildSizesInfo.accelerationStructureSize, BufferUsage::AccelerationStructureStorage);

		accelerationStructureInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.createFlags = 0,
			.buffer = accelerationStructure->TopLevelAccelerationStructureStorage->Handle,
			.offset = 0,
			.size = buildSizesInfo.accelerationStructureSize,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		};
		vkCreateAccelerationStructureKHR(context->Device, &accelerationStructureInfo, nullptr, &accelerationStructure->TopLevelAS);

		scratchBuffer = Buffer::Create(context, buildSizesInfo.buildScratchSize + accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment, BufferUsage::Storage);

		buildInfo.dstAccelerationStructure = accelerationStructure->TopLevelAS;
		buildInfo.scratchData.deviceAddress = AlignUp(scratchBuffer->Address, Cast<uint64_t>(accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment));

		VkAccelerationStructureBuildRangeInfoKHR BuildOffsets{ numInstances, 0, 0, 0 };
		buildOffsetsPtr = &BuildOffsets;
		vkCmdBuildAccelerationStructuresKHR(cmdList->Handle, 1, &buildInfo, &buildOffsetsPtr);

		cmdList.End();

		queue.Submit({ cmdList }, {}, { fence });

		fence.Wait();

		scratchBuffer.Destroy();

		fence.Destroy();
		cmdPool.Destroy();

		return { accelerationStructure };
	}

	uint64_t AccelerationStructure::GetTopLevelAddress()
	{
		VkAccelerationStructureDeviceAddressInfoKHR addressInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = m_Impl->TopLevelAS
		};
		return vkGetAccelerationStructureDeviceAddressKHR(m_Impl->Ctx->Device, &addressInfo);
	}

	/*void VulkanRenderDevice::AccelerationStructureDestroy(AccelerationStructureRH InAccelerationStructure)
	{
	}*/

}
