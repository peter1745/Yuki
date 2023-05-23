#pragma once

#include "Vulkan.hpp"

#include <vma/vk_mem_alloc.h>

namespace Yuki {

	class VulkanAllocator
	{
	public:
		VkImage CreateImage(const VkImageCreateInfo* InCreateInfo, VmaAllocation* OutAllocation);
		void DestroyImage(VkImage InImage, VmaAllocation InAllocation);

	private:
		void Initialize(VkInstance InInstance, VkPhysicalDevice InPhysicalDevice, VkDevice InDevice);
		void Destroy();

	private:
		VmaAllocator m_Allocator;

	private:
		friend class VulkanRenderContext;
	};

}
