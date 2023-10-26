#include "VulkanRHI.hpp"

#include "Features/VulkanRaytracingFeature.hpp"

namespace Yuki::RHI {

	static constexpr uint32_t MaxInstances = 65536;
	static constexpr uint32_t InstanceBufferSize = MaxInstances * sizeof(VkAccelerationStructureInstanceKHR);

	AccelerationStructure AccelerationStructure::Create(Context context)
	{
		auto accelerationStructure = new Impl();
		accelerationStructure->Ctx = context;

		accelerationStructure->InstancesBuffer = Buffer::Create(context, InstanceBufferSize, BufferUsage::AccelerationStructureBuildInput, BufferFlags::Mapped | BufferFlags::DeviceLocal);

		return { accelerationStructure };
	}

	GeometryID AccelerationStructure::AddGeometry(Span<Vec3> vertexPositions, Span<uint32_t> indices)
	{
		auto geometryBuffer = Buffer::Create(m_Impl->Ctx,
											vertexPositions.ByteSize(),
											BufferUsage::Storage |
											BufferUsage::AccelerationStructureBuildInput |
											BufferUsage::TransferDst,
											BufferFlags::Mapped | BufferFlags::DeviceLocal);

		auto indexBuffer = Buffer::Create(m_Impl->Ctx,
										indices.ByteSize(),
										BufferUsage::Index |
										BufferUsage::AccelerationStructureBuildInput |
										BufferUsage::TransferDst,
										BufferFlags::Mapped | BufferFlags::DeviceLocal);

		geometryBuffer.Set(vertexPositions);
		indexBuffer.Set(indices);

		VkAccelerationStructureGeometryTrianglesDataKHR trianglesData =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
			.pNext = nullptr,
			.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
			.vertexData = { .deviceAddress = geometryBuffer->Address },
			.vertexStride = sizeof(Vec3),
			.maxVertex = Cast<uint32_t>(vertexPositions.Count() - 1),
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
			//.flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
		};

		uint32_t numPrimitives = Cast<uint32_t>(indices.Count() / 3);

		VkAccelerationStructureBuildRangeInfoKHR offset =
		{
			.primitiveCount = numPrimitives,
			.primitiveOffset = 0,
			.firstVertex = 0,
			.transformOffset = 0,
		};

		VkAccelerationStructureBuildGeometryInfoKHR buildInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.pNext = nullptr,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR |
					VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR |
					VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DATA_ACCESS_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.geometryCount = 1,
			.pGeometries = &geometry,
		};

		VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		vkGetAccelerationStructureBuildSizesKHR(m_Impl->Ctx->Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &numPrimitives, &buildSizesInfo);

		const auto& accelerationStructureProperties = m_Impl->Ctx->GetFeature<VulkanRaytracingFeature>().GetAccelerationStructureProperties();

		auto scratchBuffer = Buffer::Create(m_Impl->Ctx, buildSizesInfo.buildScratchSize + accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment, BufferUsage::Storage);

		VkQueryPool queryPool;
		VkQueryPoolCreateInfo queryPoolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
			.queryCount = 1,
		};
		YUKI_VK_CHECK(vkCreateQueryPool(m_Impl->Ctx->Device, &queryPoolInfo, nullptr, &queryPool));

		GeometryID geometryID;
		auto& blas = m_Impl->BottomLevelStructures[geometryID];

		blas.StructureStorage = Buffer::Create(m_Impl->Ctx, buildSizesInfo.accelerationStructureSize, BufferUsage::AccelerationStructureStorage);

		VkAccelerationStructureCreateInfoKHR accelerationStructureInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.createFlags = 0,
			.buffer = blas.StructureStorage->Handle,
			.offset = 0,
			.size = buildSizesInfo.accelerationStructureSize,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		};
		YUKI_VK_CHECK(vkCreateAccelerationStructureKHR(m_Impl->Ctx->Device, &accelerationStructureInfo, nullptr, &blas.Structure));

		buildInfo.dstAccelerationStructure = blas.Structure;
		buildInfo.scratchData.deviceAddress = AlignUp(scratchBuffer->Address, Cast<uint64_t>(accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment));

		auto cmd = m_Impl->Ctx->GetTemporaryCommandList();

		const auto* buildOffsetsPtr = &offset;
		vkCmdBuildAccelerationStructuresKHR(cmd->Handle, 1, &buildInfo, &buildOffsetsPtr);

		m_Impl->Ctx->EndTemporaryCommandList(cmd);

		VkAccelerationStructureKHR originalBlas = blas.Structure;
		Buffer originalStorage = blas.StructureStorage;

		{
			cmd = m_Impl->Ctx->GetTemporaryCommandList();
			vkCmdResetQueryPool(cmd->Handle, queryPool, 0, 1);
			vkCmdWriteAccelerationStructuresPropertiesKHR(cmd->Handle, 1, &blas.Structure, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, queryPool, 0);
			m_Impl->Ctx->EndTemporaryCommandList(cmd);

			VkDeviceSize compactSize;
			vkGetQueryPoolResults(m_Impl->Ctx->Device, queryPool, 0, 1, sizeof(VkDeviceSize), &compactSize, sizeof(VkDeviceSize), VK_QUERY_RESULT_64_BIT);

			blas.StructureStorage = Buffer::Create(m_Impl->Ctx, compactSize, BufferUsage::AccelerationStructureStorage);
			accelerationStructureInfo.buffer = blas.StructureStorage->Handle;
			accelerationStructureInfo.size = compactSize;

			YUKI_VK_CHECK(vkCreateAccelerationStructureKHR(m_Impl->Ctx->Device, &accelerationStructureInfo, nullptr, &blas.Structure));

			cmd = m_Impl->Ctx->GetTemporaryCommandList();
			VkCopyAccelerationStructureInfoKHR copyInfo =
			{
				.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR,
				.src = originalBlas,
				.dst = blas.Structure,
				.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR,
			};
			vkCmdCopyAccelerationStructureKHR(cmd->Handle, &copyInfo);
			m_Impl->Ctx->EndTemporaryCommandList(cmd);
		}

		vkDestroyAccelerationStructureKHR(m_Impl->Ctx->Device, originalBlas, nullptr);
		originalStorage.Destroy();
		vkDestroyQueryPool(m_Impl->Ctx->Device, queryPool, nullptr);

		scratchBuffer.Destroy();
		geometryBuffer.Destroy();
		indexBuffer.Destroy();

		return geometryID;
	}

	void AccelerationStructure::AddInstance(GeometryID geometry, const Mat4& transform, uint32_t customInstanceIndex, uint32_t sbtOffset)
	{
		YUKI_VERIFY(m_Impl->InstanceCount < MaxInstances);

		VkAccelerationStructureDeviceAddressInfoKHR addressInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = m_Impl->BottomLevelStructures[geometry].Structure,
		};

		VkAccelerationStructureInstanceKHR instance =
		{
			.transform = {
				.matrix = {
					transform[0][0], transform[1][0], transform[2][0], transform[3][0],
					transform[0][1], transform[1][1], transform[2][1], transform[3][1],
					transform[0][2], transform[1][2], transform[2][2], transform[3][2],
				}
			},
			.instanceCustomIndex = customInstanceIndex,
			.mask = 0xFF,
			.instanceShaderBindingTableRecordOffset = sbtOffset,
			.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
			.accelerationStructureReference = vkGetAccelerationStructureDeviceAddressKHR(m_Impl->Ctx->Device, &addressInfo),
		};

		m_Impl->InstancesBuffer.Set<VkAccelerationStructureInstanceKHR>({ instance }, m_Impl->InstanceCount);
		m_Impl->InstanceCount++;

		m_Impl->RebuildTopLevelStructure();
	}

	void AccelerationStructure::Impl::RebuildTopLevelStructure()
	{
		VkAccelerationStructureGeometryInstancesDataKHR instancesData =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.pNext = nullptr,
			.arrayOfPointers = VK_FALSE,
			.data = {
				.deviceAddress = InstancesBuffer.GetDeviceAddress()
			},
		};

		VkAccelerationStructureGeometryKHR geometry =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
			.pNext = nullptr,
			.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
			.geometry = {
				.instances = instancesData
			},
			.flags = 0,
		};

		VkAccelerationStructureBuildGeometryInfoKHR buildInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
			.pNext = nullptr,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
			.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
			.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
			.geometryCount = 1,
			.pGeometries = &geometry,
		};

		VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		vkGetAccelerationStructureBuildSizesKHR(Ctx->Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &InstanceCount, &buildSizesInfo);

		if (TopLevelStructure)
		{
			vkDestroyAccelerationStructureKHR(Ctx->Device, TopLevelStructure, nullptr);
			TopLevelStructureStorage.Destroy();
		}

		TopLevelStructureStorage = Buffer::Create(Ctx, buildSizesInfo.accelerationStructureSize, BufferUsage::AccelerationStructureStorage);

		VkAccelerationStructureCreateInfoKHR accelerationStructureInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.createFlags = 0,
			.buffer = TopLevelStructureStorage->Handle,
			.offset = 0,
			.size = buildSizesInfo.accelerationStructureSize,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		};
		vkCreateAccelerationStructureKHR(Ctx->Device, &accelerationStructureInfo, nullptr, &TopLevelStructure);

		const auto& accelerationStructureProperties = Ctx->GetFeature<VulkanRaytracingFeature>().GetAccelerationStructureProperties();

		auto scratchBuffer = Buffer::Create(Ctx, buildSizesInfo.buildScratchSize + accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment, BufferUsage::Storage);

		buildInfo.dstAccelerationStructure = TopLevelStructure;
		buildInfo.scratchData.deviceAddress = AlignUp(scratchBuffer->Address, Cast<uint64_t>(accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment));

		VkAccelerationStructureBuildRangeInfoKHR buildOffsets{ InstanceCount, 0, 0, 0 };
		const auto* buildOffsetsPtr = &buildOffsets;

		auto cmd = Ctx->GetTemporaryCommandList();
		vkCmdBuildAccelerationStructuresKHR(cmd->Handle, 1, &buildInfo, &buildOffsetsPtr);
		Ctx->EndTemporaryCommandList(cmd);

		scratchBuffer.Destroy();
	}

	uint64_t AccelerationStructure::GetTopLevelAddress()
	{
		VkAccelerationStructureDeviceAddressInfoKHR addressInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			.accelerationStructure = m_Impl->TopLevelStructure
		};
		return vkGetAccelerationStructureDeviceAddressKHR(m_Impl->Ctx->Device, &addressInfo);
	}

	/*void VulkanRenderDevice::AccelerationStructureDestroy(AccelerationStructureRH InAccelerationStructure)
	{
	}*/

}
