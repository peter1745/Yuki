#pragma once

#include "VulkanInclude.hpp"

namespace Yuki::RHI {

	struct VulkanFence
	{
		VkSemaphore Handle = VK_NULL_HANDLE;
		uint64_t Value = 0;
	};

}
