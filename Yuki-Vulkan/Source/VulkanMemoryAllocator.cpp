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

	ImageAllocation VulkanMemoryAllocator::CreateImage(const VkImageCreateInfo& createInfo) const
	{
		VmaAllocationCreateInfo allocationInfo =
		{
			.usage = VMA_MEMORY_USAGE_AUTO
		};

		ImageAllocation allocation{};
		Vulkan::CheckResult(vmaCreateImage(m_Impl->Allocator, &createInfo, &allocationInfo, &allocation.Image, &allocation.Allocation, nullptr));
		return allocation;
	}

	void VulkanMemoryAllocator::DestroyImage(const ImageAllocation& allocation) const
	{
		vmaDestroyImage(m_Impl->Allocator, allocation.Image, allocation.Allocation);
	}

}
