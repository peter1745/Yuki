#pragma once

#include <Engine/Core/Handle.hpp>

#include <vma/vk_mem_alloc.h>

namespace Yuki {

	struct VulkanMemoryAllocator : Handle<VulkanMemoryAllocator>
	{
		static VulkanMemoryAllocator Create(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
		void Destroy();
	};

}
