#pragma once

#include <Engine/RHI/RHI.hpp>

#include <vma/vk_mem_alloc.h>

namespace Yuki {

	template<typename T>
	struct GPUAllocation
	{
		T Resource;
		VmaAllocation Allocation;
		VmaAllocationInfo AllocationInfo;
	};

	struct VulkanMemoryAllocator : Handle<VulkanMemoryAllocator>
	{
		static VulkanMemoryAllocator Create(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
		void Destroy();

		GPUAllocation<VkImage> CreateImage(const VkImageCreateInfo& createInfo) const;
		void DestroyImage(const GPUAllocation<VkImage>& allocation) const;

		GPUAllocation<VkBuffer> CreateBuffer(const VkBufferCreateInfo& createInfo, BufferUsage usage) const;
		void DestroyBuffer(const GPUAllocation<VkBuffer>& allocation) const;
	};

}
