#pragma once

#include <Engine/RHI/RHI.hpp>

#include <vma/vk_mem_alloc.h>

namespace Yuki {

	struct ImageAllocation
	{
		VkImage Image;
		VmaAllocation Allocation;
	};

	struct VulkanMemoryAllocator : Handle<VulkanMemoryAllocator>
	{
		static VulkanMemoryAllocator Create(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
		void Destroy();

		ImageAllocation CreateImage(const VkImageCreateInfo& createInfo) const;
		void DestroyImage(const ImageAllocation& allocation) const;
	};

}
