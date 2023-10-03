#pragma once

#include "VulkanInclude.hpp"

namespace Yuki::RHI {

	struct VulkanQueue
	{
		VkQueue Handle = VK_NULL_HANDLE;
		uint32_t Family = std::numeric_limits<uint32_t>::max();
		uint32_t Index = 0;
		VkQueueFlags Flags = 0;
	};

}
