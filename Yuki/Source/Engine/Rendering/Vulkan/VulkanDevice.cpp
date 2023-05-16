#include "VulkanDevice.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	VulkanDevice::VulkanDevice(VkPhysicalDevice InPhysicalDevice)
	    : m_PhysicalDevice(InPhysicalDevice)
	{
		CalculateDeviceScore();
	}

	void VulkanDevice::WaitIdle() const
	{
		vkDeviceWaitIdle(m_Device);
	}

	void VulkanDevice::CreateLogicalDevice(VkSurfaceKHR InSurface, const List<const char*>& InDeviceLayers)
	{
		uint32_t selectedQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
		uint32_t currentQueueFamilyIndex = 0;

		List<VkQueueFamilyProperties> queueFamilyProperties;
		VulkanHelper::Enumerate(vkGetPhysicalDeviceQueueFamilyProperties, queueFamilyProperties, m_PhysicalDevice);
		for (const auto& queueFamily : queueFamilyProperties)
		{
			VkBool32 canPresentToSurface = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, currentQueueFamilyIndex, InSurface, &canPresentToSurface);

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && canPresentToSurface == VK_TRUE)
			{
				selectedQueueFamilyIndex = currentQueueFamilyIndex;
				break;
			}

			currentQueueFamilyIndex++;
		}

		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = selectedQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		};

		VkDeviceCreateInfo deviceInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledLayerCount = uint32_t(InDeviceLayers.Count()),
			.ppEnabledLayerNames = InDeviceLayers.Data(),
			.enabledExtensionCount = 0,
			.ppEnabledExtensionNames = nullptr,
			.pEnabledFeatures = nullptr,
		};

		YUKI_VERIFY(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device) == VK_SUCCESS);
		volkLoadDevice(m_Device);

		vkGetDeviceQueue(m_Device, selectedQueueFamilyIndex, 0, &m_Queue);
		YUKI_VERIFY(m_Queue != VK_NULL_HANDLE);
	}

	void VulkanDevice::Destroy()
	{
		vkDestroyDevice(m_Device, nullptr);
	}

	void VulkanDevice::CalculateDeviceScore()
	{
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties);

		switch (m_PhysicalDeviceProperties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		{
			m_DeviceScore += 5;
			break;
		}
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		{
			m_DeviceScore += 10;
			break;
		}
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		{
			m_DeviceScore += 3;
			break;
		}
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
		{
			m_DeviceScore += 1;
			break;
		}
		default:
		{
			YUKI_VERIFY(false, "Unsupported Physical Device!");
			break;
		}
		}

		VkPhysicalDeviceVulkan13Features deviceFeatures13 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		VkPhysicalDeviceFeatures2 deviceFeatures2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &deviceFeatures13 };
		vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &deviceFeatures2);

		if (deviceFeatures13.dynamicRendering == VK_TRUE)
			m_DeviceScore += 10;

		if (deviceFeatures13.synchronization2 == VK_TRUE)
			m_DeviceScore += 10;
	}

}
