#include "VulkanAllocator.hpp"

namespace Yuki {

	VkImage VulkanAllocator::CreateImage(const VkImageCreateInfo* InCreateInfo, VmaAllocation* OutAllocation)
	{
		VkImage result = VK_NULL_HANDLE;
		VmaAllocationCreateInfo allocationInfo = { .usage = VMA_MEMORY_USAGE_AUTO };
		vmaCreateImage(m_Allocator, InCreateInfo, &allocationInfo, &result, OutAllocation, nullptr);
		return result;
	}

	void VulkanAllocator::DestroyImage(VkImage InImage, VmaAllocation InAllocation)
	{
		vmaDestroyImage(m_Allocator, InImage, InAllocation);
	}

	VkBuffer VulkanAllocator::CreateBuffer(BufferType InBufferType, const VkBufferCreateInfo* InCreateInfo, VmaAllocation* OutAllocation)
	{
		VkBuffer result = VK_NULL_HANDLE;
		VmaAllocationCreateInfo allocationInfo = { .usage = VMA_MEMORY_USAGE_AUTO, };

		if (InBufferType == BufferType::StagingBuffer)
		{
			allocationInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		}

		vmaCreateBuffer(m_Allocator, InCreateInfo, &allocationInfo, &result, OutAllocation, nullptr);
		return result;
	}

	void VulkanAllocator::DestroyBuffer(VkBuffer InBuffer, VmaAllocation InAllocation)
	{
		vmaDestroyBuffer(m_Allocator, InBuffer, InAllocation);
	}

	void* VulkanAllocator::MapMemory(VmaAllocation InAllocation)
	{
		void* mappedMemory = nullptr;
		vmaMapMemory(m_Allocator, InAllocation, &mappedMemory);
		return mappedMemory;
	}

	void VulkanAllocator::UnmapMemory(VmaAllocation InAllocation)
	{
		vmaUnmapMemory(m_Allocator, InAllocation);
	}

	void VulkanAllocator::Initialize(VkInstance InInstance, VkPhysicalDevice InPhysicalDevice, VkDevice InDevice)
	{
		VmaVulkanFunctions vmaVulkanFunctions = {};
		vmaVulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
		vmaVulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;

		VmaAllocatorCreateInfo allocatorCreateInfo =
		{
			.physicalDevice = InPhysicalDevice,
			.device = InDevice,
			.pVulkanFunctions = &vmaVulkanFunctions,
			.instance = InInstance,
			.vulkanApiVersion = VK_API_VERSION_1_3,
		};

		vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator);
	}

	void VulkanAllocator::Destroy()
	{
		vmaDestroyAllocator(m_Allocator);
	}

}
