#pragma once

#include "VulkanInclude.hpp"

namespace Yuki {

	struct VulkanFence
	{
		VkSemaphore Semaphore = VK_NULL_HANDLE;
		uint64_t Value = 0;
	};

}
