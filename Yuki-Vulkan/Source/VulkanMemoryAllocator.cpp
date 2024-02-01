#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_IMPLEMENTATION
#include "VulkanMemoryAllocator.hpp"

#include "VulkanCommon.hpp"

namespace Yuki {

	template<>
	struct Handle<VulkanMemoryAllocator>::Impl
	{
		VmaAllocator Allocator;
	};

	VulkanMemoryAllocator VulkanMemoryAllocator::Create(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
	{
		auto* impl = new Impl();

		VmaVulkanFunctions vulkanFunctions = {};
		vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorInfo =
		{
			.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
			.physicalDevice = physicalDevice,
			.device = device,
			.pVulkanFunctions = &vulkanFunctions,
			.instance = instance,
			.vulkanApiVersion = VK_API_VERSION_1_3,
		};

		Vulkan::CheckResult(vmaCreateAllocator(&allocatorInfo, &impl->Allocator));

		return { impl };
	}

	void VulkanMemoryAllocator::Destroy()
	{
		vmaDestroyAllocator(m_Impl->Allocator);
		delete m_Impl;
	}

	GPUAllocation<VkImage> VulkanMemoryAllocator::CreateImage(const VkImageCreateInfo& createInfo) const
	{
		VmaAllocationCreateInfo allocationInfo =
		{
			.usage = VMA_MEMORY_USAGE_AUTO
		};

		GPUAllocation<VkImage> allocation{};
		Vulkan::CheckResult(vmaCreateImage(
			m_Impl->Allocator,
			&createInfo,
			&allocationInfo,
			&allocation.Resource,
			&allocation.Allocation,
			&allocation.AllocationInfo
		));
		return allocation;
	}

	void VulkanMemoryAllocator::DestroyImage(const GPUAllocation<VkImage>& allocation) const
	{
		vmaDestroyImage(m_Impl->Allocator, allocation.Resource, allocation.Allocation);
	}

	GPUAllocation<VkBuffer> VulkanMemoryAllocator::CreateBuffer(const VkBufferCreateInfo& createInfo, BufferUsage usage) const
	{
		VmaAllocationCreateInfo allocationInfo =
		{
			.usage = VMA_MEMORY_USAGE_AUTO,
		};

		if (usage & BufferUsage::Mapped)
		{
			allocationInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			if (usage & BufferUsage::DeviceLocal)
			{
				allocationInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
				allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			}
			else
			{
				allocationInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			}
		}

		GPUAllocation<VkBuffer> allocation{};
		Vulkan::CheckResult(vmaCreateBuffer(
			m_Impl->Allocator,
			&createInfo,
			&allocationInfo,
			&allocation.Resource,
			&allocation.Allocation,
			&allocation.AllocationInfo
		));
		return allocation;
	}

	void VulkanMemoryAllocator::DestroyBuffer(const GPUAllocation<VkBuffer>& allocation) const
	{
		vmaDestroyBuffer(m_Impl->Allocator, allocation.Resource, allocation.Allocation);
	}


}
