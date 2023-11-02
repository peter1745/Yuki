#include "VulkanRHI.hpp"

#include "Features/VulkanRaytracingFeature.hpp"

#include "Engine/Common/Timer.hpp"

namespace Yuki::RHI {

	static constexpr uint32_t MaxInstances = 65536;
	static constexpr uint32_t InstanceBufferSize = MaxInstances * sizeof(VkAccelerationStructureInstanceKHR);

	AccelerationStructureBuilder AccelerationStructureBuilder::Create(Context context)
	{
		auto builder = new Impl();
		builder->Ctx = context;

		VkQueryPoolCreateInfo queryPoolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
			.queryCount = 2000,
		};
		YUKI_VK_CHECK(vkCreateQueryPool(context->Device, &queryPoolInfo, nullptr, &builder->QueryPool));

		return { builder };
	}

	BlasID AccelerationStructureBuilder::CreateBLAS() const
	{
		BlasID id = m_Impl->BottomLevelStructures.size();
		m_Impl->BottomLevelStructures.push_back({});
		return id;
	}

	GeometryID AccelerationStructureBuilder::AddGeometry(BlasID blas, Span<Vec3> vertexPositions, Buffer indexBuffer, uint32_t indexCount, bool isOpaque) const
	{
		YUKI_VERIFY(blas < m_Impl->BottomLevelStructures.size());

		auto& blasData = m_Impl->BottomLevelStructures[blas];

		GeometryID geometryID = blasData.Geometries.size();
		auto& geometryData = blasData.Geometries.emplace_back();

		RHI::Buffer vertexBuffer = RHI::Buffer::Create(m_Impl->Ctx, vertexPositions.ByteSize(),
			RHI::BufferUsage::AccelerationStructureBuildInput,
			RHI::BufferFlags::Mapped |
			RHI::BufferFlags::DeviceLocal);
		vertexBuffer.Set(vertexPositions);

		blasData.VertexBuffers.push_back(vertexBuffer);
		blasData.IndexBuffers.push_back({
			.IndexBuffer = indexBuffer,
			.IndexCount = indexCount
		});

		VkAccelerationStructureGeometryTrianglesDataKHR trianglesData =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
			.pNext = nullptr,
			.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
			.vertexData = { .deviceAddress = vertexBuffer->Address },
			.vertexStride = sizeof(Vec3),
			.maxVertex = vertexPositions.Count() - 1,
			.indexType = VK_INDEX_TYPE_UINT32,
			.indexData = { .deviceAddress = indexBuffer->Address },
			.transformData = {},
		};

		geometryData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometryData.pNext = nullptr;
		geometryData.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometryData.geometry = {
			.triangles = trianglesData
		};

		if (isOpaque)
			geometryData.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

		return geometryID;
	}

	void AccelerationStructureBuilder::AddInstance(BlasID blas, GeometryID geometry, const Mat4& transform, uint32_t customInstanceIndex, uint32_t sbtOffset) const
	{
		YUKI_VERIFY(m_Impl->Instances.size() < MaxInstances);

		m_Impl->Instances.push_back({
			.BLAS = blas,
			.Geometry = geometry,
			.Transform = transform,
			.CustomInstanceIndex = customInstanceIndex,
			.SBTOffset = sbtOffset
		});
	}

	void AccelerationStructureBuilder::Impl::BuildBottomLevelStructures(AccelerationStructure::Impl* accelerationStructure)
	{
		const auto& accelerationStructureProperties = Ctx->GetFeature<VulkanRaytracingFeature>().GetAccelerationStructureProperties();

		DynamicArray<VkAccelerationStructureKHR> blases;
		DynamicArray<VkAccelerationStructureBuildGeometryInfoKHR> buildInfos;
		buildInfos.reserve(BottomLevelStructures.size());

		DynamicArray<VkAccelerationStructureBuildRangeInfoKHR*> buildRanges;
		DynamicArray<Buffer> scratchBuffers;

		uint64_t totalBlasSize = 0;
		Timer::Start();
		for (const auto& blasData : BottomLevelStructures)
		{
			auto& buildInfo = buildInfos.emplace_back();
			buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			buildInfo.pNext = nullptr;
			buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			buildInfo.flags =
				VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR |
				VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR |
				VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_DATA_ACCESS_KHR;
			buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
			buildInfo.geometryCount = Cast<uint32_t>(blasData.Geometries.size());
			buildInfo.pGeometries = blasData.Geometries.data();

			auto* blasBuildRanges = new VkAccelerationStructureBuildRangeInfoKHR[blasData.Geometries.size()];

			DynamicArray<uint32_t> primitiveCounts;
			primitiveCounts.resize(blasData.Geometries.size());
			for (size_t i = 0; i < blasData.Geometries.size(); i++)
			{
				const auto& indexData = blasData.IndexBuffers[i];
				blasBuildRanges[i] = {
					.primitiveCount = indexData.IndexCount / 3,
					.primitiveOffset = 0,
					.firstVertex = 0,
					.transformOffset = 0,
				};
				primitiveCounts[i] = indexData.IndexCount;
			}
			buildRanges.push_back(blasBuildRanges);

			VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
			vkGetAccelerationStructureBuildSizesKHR(Ctx->Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, primitiveCounts.data(), &buildSizesInfo);

			uint64_t scratchSize = buildSizesInfo.buildScratchSize + accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment;
			auto scratchBuffer = Buffer::Create(Ctx, scratchSize, BufferUsage::Storage);

			uint64_t blasSize = buildSizesInfo.accelerationStructureSize;
			// TODO(Peter): Use blasSize to determine if a dedicated storage buffer is needed, or if we can pack this into an existing one

			auto& blasStorage = accelerationStructure->BottomLevelStructures.emplace_back();
			blasStorage.Storage = Buffer::Create(Ctx, blasSize, BufferUsage::AccelerationStructureStorage);

			VkAccelerationStructureCreateInfoKHR accelerationStructureInfo =
			{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
				.pNext = nullptr,
				.createFlags = 0,
				.buffer = blasStorage.Storage->Handle,
				.offset = 0,
				.size = blasSize,
				.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			};
			YUKI_VK_CHECK(vkCreateAccelerationStructureKHR(Ctx->Device, &accelerationStructureInfo, nullptr, &blasStorage.Structure));

			blases.push_back(blasStorage.Structure);

			buildInfo.dstAccelerationStructure = blasStorage.Structure;
			buildInfo.scratchData.deviceAddress = AlignUp(scratchBuffer->Address, Cast<uint64_t>(accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment));

			scratchBuffers.push_back(scratchBuffer);

			totalBlasSize += blasSize;
		}
		Timer::Stop("BLAS Building");

		// Build the blases
		auto cmd = Ctx->GetTemporaryCommandList();
		vkCmdBuildAccelerationStructuresKHR(cmd->Handle, Cast<uint32_t>(buildInfos.size()), buildInfos.data(), buildRanges.data());
		Ctx->EndTemporaryCommandList(cmd);

		// Free build ranges
		for (auto* blasBuildRanges : buildRanges)
			delete[] blasBuildRanges;
		buildRanges.clear();

		Logging::Info("Built {} BLASs with a total size of: {}Mb ({} bytes)", blases.size(), totalBlasSize / (1024 * 1024), totalBlasSize);

		// Fetch the real sizes of the blases
		DynamicArray<VkDeviceSize> blasSizes(blases.size());
		{
			cmd = Ctx->GetTemporaryCommandList();
			vkCmdResetQueryPool(cmd->Handle, QueryPool, 0, Cast<uint32_t>(blasSizes.size()));
			vkCmdWriteAccelerationStructuresPropertiesKHR(cmd->Handle, Cast<uint32_t>(blases.size()), blases.data(), VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, QueryPool, 0);
			Ctx->EndTemporaryCommandList(cmd);

			vkGetQueryPoolResults(Ctx->Device, QueryPool, 0, Cast<uint32_t>(blasSizes.size()), Cast<uint32_t>(blasSizes.size() * sizeof(VkDeviceSize)), blasSizes.data(), sizeof(VkDeviceSize), VK_QUERY_RESULT_64_BIT);
		}

		Logging::Info("Compressing BLASs");

		totalBlasSize = 0;

		DynamicArray<VkAccelerationStructureKHR> oldAccelerationStructures;
		oldAccelerationStructures.resize(accelerationStructure->BottomLevelStructures.size());

		DynamicArray<Buffer> oldStructureStorageBuffers;
		oldStructureStorageBuffers.resize(accelerationStructure->BottomLevelStructures.size());

		// BLAS Compression
		Timer::Start();
		cmd = Ctx->GetTemporaryCommandList();
		for (size_t i = 0; i < accelerationStructure->BottomLevelStructures.size(); i++)
		{
			auto& blasStorage = accelerationStructure->BottomLevelStructures[i];

			oldAccelerationStructures[i] = blasStorage.Structure;
			oldStructureStorageBuffers[i] = blasStorage.Storage;

			blasStorage.Storage = Buffer::Create(Ctx, blasSizes[i], BufferUsage::AccelerationStructureStorage);
			VkAccelerationStructureCreateInfoKHR accelerationStructureInfo =
			{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
				.pNext = nullptr,
				.createFlags = 0,
				.buffer = blasStorage.Storage->Handle,
				.offset = 0,
				.size = blasSizes[i],
				.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
			};
			YUKI_VK_CHECK(vkCreateAccelerationStructureKHR(Ctx->Device, &accelerationStructureInfo, nullptr, &blasStorage.Structure));

			VkCopyAccelerationStructureInfoKHR copyInfo =
			{
				.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR,
				.src = oldAccelerationStructures[i],
				.dst = blasStorage.Structure,
				.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR,
			};
			vkCmdCopyAccelerationStructureKHR(cmd->Handle, &copyInfo);

			totalBlasSize += blasSizes[i];
		}
		Ctx->EndTemporaryCommandList(cmd);

		for (size_t i = 0; i < accelerationStructure->BottomLevelStructures.size(); i++)
		{
			vkDestroyAccelerationStructureKHR(Ctx->Device, oldAccelerationStructures[i], nullptr);
			oldStructureStorageBuffers[i].Destroy();
		}
		Timer::Stop("BLAS Compression");

		Logging::Info("Size after compression = {}Mb ({} bytes)", totalBlasSize / (1024 * 1024), totalBlasSize);

		// Free up memory used for BLAS building
		for (auto& blasData : BottomLevelStructures)
		{
			for (auto vertexBuffer : blasData.VertexBuffers)
				vertexBuffer.Destroy();
		}

		for (auto scratchBuffer : scratchBuffers)
			scratchBuffer.Destroy();
	}

	AccelerationStructure AccelerationStructureBuilder::Build() const
	{
		AccelerationStructure::Impl* accelerationStructure = new AccelerationStructure::Impl();
		accelerationStructure->Ctx = m_Impl->Ctx;
		accelerationStructure->InstancesBuffer = Buffer::Create(m_Impl->Ctx, InstanceBufferSize, BufferUsage::AccelerationStructureBuildInput, BufferFlags::Mapped | BufferFlags::DeviceLocal);

		m_Impl->BuildBottomLevelStructures(accelerationStructure);
		m_Impl->BuildTopLevelStructure(accelerationStructure);

		return { accelerationStructure };
	}

	void AccelerationStructureBuilder::Impl::BuildTopLevelStructure(AccelerationStructure::Impl* accelerationStructure)
	{
		Timer::Start();

		uint32_t instanceIndex = 0;
		for (const auto& instanceData : Instances)
		{
			VkAccelerationStructureDeviceAddressInfoKHR addressInfo =
			{
				.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
				.accelerationStructure = accelerationStructure->BottomLevelStructures[instanceData.BLAS].Structure,
			};

			accelerationStructure->InstancesBuffer.Set<VkAccelerationStructureInstanceKHR>({
				{
					.transform = {
						.matrix = {
							instanceData.Transform[0][0], instanceData.Transform[1][0], instanceData.Transform[2][0], instanceData.Transform[3][0],
							instanceData.Transform[0][1], instanceData.Transform[1][1], instanceData.Transform[2][1], instanceData.Transform[3][1],
							instanceData.Transform[0][2], instanceData.Transform[1][2], instanceData.Transform[2][2], instanceData.Transform[3][2],
						}
					},
					.instanceCustomIndex = instanceData.CustomInstanceIndex,
					.mask = 0xFF,
					.instanceShaderBindingTableRecordOffset = instanceData.SBTOffset,
					.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
					.accelerationStructureReference = vkGetAccelerationStructureDeviceAddressKHR(Ctx->Device, &addressInfo),
				}
			}, instanceIndex++);
		}

		VkAccelerationStructureGeometryInstancesDataKHR instancesData =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			.pNext = nullptr,
			.arrayOfPointers = VK_FALSE,
			.data = {
				.deviceAddress = accelerationStructure->InstancesBuffer.GetDeviceAddress()
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

		uint32_t instanceCount = Cast<uint32_t>(Instances.size());

		VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
		vkGetAccelerationStructureBuildSizesKHR(Ctx->Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &instanceCount, &buildSizesInfo);

		if (accelerationStructure->TopLevelStructure)
		{
			vkDestroyAccelerationStructureKHR(Ctx->Device, accelerationStructure->TopLevelStructure, nullptr);
			accelerationStructure->TopLevelStructureStorage.Destroy();
		}

		accelerationStructure->TopLevelStructureStorage = Buffer::Create(Ctx, buildSizesInfo.accelerationStructureSize, BufferUsage::AccelerationStructureStorage);

		VkAccelerationStructureCreateInfoKHR accelerationStructureInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.createFlags = 0,
			.buffer = accelerationStructure->TopLevelStructureStorage->Handle,
			.offset = 0,
			.size = buildSizesInfo.accelerationStructureSize,
			.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		};
		vkCreateAccelerationStructureKHR(Ctx->Device, &accelerationStructureInfo, nullptr, &accelerationStructure->TopLevelStructure);

		const auto& accelerationStructureProperties = Ctx->GetFeature<VulkanRaytracingFeature>().GetAccelerationStructureProperties();

		auto scratchBuffer = Buffer::Create(Ctx, buildSizesInfo.buildScratchSize + accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment, BufferUsage::Storage);

		buildInfo.dstAccelerationStructure = accelerationStructure->TopLevelStructure;
		buildInfo.scratchData.deviceAddress = AlignUp(scratchBuffer->Address, Cast<uint64_t>(accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment));

		VkAccelerationStructureBuildRangeInfoKHR buildOffsets{ instanceCount, 0, 0, 0 };
		const auto* buildOffsetsPtr = &buildOffsets;

		auto cmd = Ctx->GetTemporaryCommandList();
		vkCmdBuildAccelerationStructuresKHR(cmd->Handle, 1, &buildInfo, &buildOffsetsPtr);
		Ctx->EndTemporaryCommandList(cmd);

		scratchBuffer.Destroy();

		Timer::Stop("TLAS Building");
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
