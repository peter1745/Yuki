#pragma once

#include "Engine/RHI/RenderHandles.hpp"

#include "VulkanInclude.hpp"

namespace Yuki::RHI {

	struct VulkanAccelerationStructure
	{
		VkAccelerationStructureKHR BottomLevelAS;
		BufferRH AccelerationStructureStorage;
		VkAccelerationStructureKHR TopLevelAS;
		BufferRH TopLevelAccelerationStructureStorage;
	};

}
