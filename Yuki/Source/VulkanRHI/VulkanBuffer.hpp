#pragma once

#include "VulkanInclude.hpp"

#include <vma/vk_mem_alloc.h>

namespace Yuki::RHI {

	struct VulkanBuffer
	{
		VkBuffer Handle = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VkDeviceAddress Address = 0;
		uint64_t Size = 0;
	};

}
