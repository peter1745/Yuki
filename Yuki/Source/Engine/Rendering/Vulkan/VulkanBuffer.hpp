#pragma once

#include "Rendering/RHI.hpp"

#include "VulkanInclude.hpp"

#include <vma/vk_mem_alloc.h>

namespace Yuki {

	struct VulkanBuffer
	{
		BufferType Type;
		uint32_t Size = 0;
		VkBufferUsageFlags UsageFlags = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
		VkBuffer Handle = VK_NULL_HANDLE;
		VmaAllocation Allocation = {};
		void* MappedMemory = nullptr;
		VkDeviceAddress DeviceAddress = UINT64_MAX;
	};

}
