#pragma once

#include "Engine/RHI/RenderHandles.hpp"
#include "Engine/RHI/PipelineInfo.hpp"

#include "VulkanInclude.hpp"

namespace Yuki::RHI {

	struct VulkanPipeline
	{
		VkPipeline Handle;
		VkPipelineLayout Layout;
	};

	struct VulkanRayTracingPipeline
	{
		VkPipeline Handle;
		VkPipelineLayout Layout;

		BufferRH SBTBuffer;
		VkStridedDeviceAddressRegionKHR RayGenRegion{};
		VkStridedDeviceAddressRegionKHR MissGenRegion{};
		VkStridedDeviceAddressRegionKHR ClosestHitGenRegion{};
		VkStridedDeviceAddressRegionKHR CallableGenRegion{};
	};

}
