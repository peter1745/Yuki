#pragma once

#include "VulkanInclude.hpp"

#include <vma/vk_mem_alloc.h>

namespace Yuki {

	class VulkanAllocator
	{
	public:
		VkImage CreateImage(const VkImageCreateInfo* InCreateInfo, VmaAllocation* OutAllocation, std::source_location location = std::source_location::current());
		void DestroyImage(VkImage InImage, VmaAllocation InAllocation);

		VkBuffer CreateBuffer(const VkBufferCreateInfo* InCreateInfo, VmaAllocation* OutAllocation, std::source_location location = std::source_location::current());
		void DestroyBuffer(VkBuffer InBuffer, VmaAllocation InAllocation);

		void* MapMemory(VmaAllocation InAllocation);
		void UnmapMemory(VmaAllocation InAllocation);

	private:
		void SetAllocationName(VmaAllocation* InAllocation, std::source_location location) const;

		void Initialize(VkInstance InInstance, VkPhysicalDevice InPhysicalDevice, VkDevice InDevice);
		void Destroy();

	private:
		VmaAllocator m_Allocator;

	private:
		friend class VulkanRenderContext;
	};

}
