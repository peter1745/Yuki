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

	VkSurfaceCapabilitiesKHR VulkanDevice::QuerySurfaceCapabilities(VkSurfaceKHR InSurface) const
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, InSurface, &surfaceCapabilities);
		return surfaceCapabilities;
	}

	void VulkanDevice::CreateLogicalDevice(const List<const char*>& InDeviceLayers)
	{
		uint32_t currentQueueFamilyIndex = 0;

		List<VkQueueFamilyProperties> queueFamilyProperties;
		VulkanHelper::Enumerate(vkGetPhysicalDeviceQueueFamilyProperties, queueFamilyProperties, m_PhysicalDevice);
		for (const auto& queueFamily : queueFamilyProperties)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				m_QueueFamilyIndex = currentQueueFamilyIndex;
				break;
			}

			currentQueueFamilyIndex++;
		}

		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_QueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		};

		List<const char*> deviceExtensions;
		deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		deviceExtensions.emplace_back(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);

		VkPhysicalDeviceVulkan13Features features13 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features13.dynamicRendering = m_DeviceFeatures13.dynamicRendering;
		features13.synchronization2 = m_DeviceFeatures13.synchronization2;

		VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extendedDynamicState3Features = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
			.pNext = &features13,
			.extendedDynamicState3PolygonMode = m_ExtendedDynamicStateDeviceFeatures.extendedDynamicState3PolygonMode,
		};

		VkPhysicalDeviceFeatures2 features2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &extendedDynamicState3Features };

		VkDeviceCreateInfo deviceInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features2,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledLayerCount = uint32_t(InDeviceLayers.size()),
			.ppEnabledLayerNames = InDeviceLayers.data(),
			.enabledExtensionCount = uint32_t(deviceExtensions.size()),
			.ppEnabledExtensionNames = deviceExtensions.data(),
			.pEnabledFeatures = nullptr,
		};

		YUKI_VERIFY(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device) == VK_SUCCESS);
		volkLoadDevice(m_Device);

		vkGetDeviceQueue(m_Device, m_QueueFamilyIndex, 0, &m_Queue);
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

		m_DeviceFeatures13 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		
		m_ExtendedDynamicStateDeviceFeatures = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
			.pNext = &m_DeviceFeatures13
		};

		m_DeviceFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &m_ExtendedDynamicStateDeviceFeatures };
		vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &m_DeviceFeatures);

		if (m_DeviceFeatures13.dynamicRendering == VK_TRUE)
			m_DeviceScore += 10;

		if (m_DeviceFeatures13.synchronization2 == VK_TRUE)
			m_DeviceScore += 10;

		if (m_ExtendedDynamicStateDeviceFeatures.extendedDynamicState3PolygonMode == VK_TRUE)
			m_DeviceScore += 10;
	}

}
