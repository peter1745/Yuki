#include "VulkanHelper.hpp"

namespace Yuki {

	uint32_t VulkanHelper::SelectGraphicsQueue(VkPhysicalDevice InPhysicalDevice)
	{
		List<VkQueueFamilyProperties> queueFamilyProperties;
		Enumerate(vkGetPhysicalDeviceQueueFamilyProperties, queueFamilyProperties, InPhysicalDevice);

		uint32_t selectedQueueIndex = 0;

		for (const auto& queueFamily : queueFamilyProperties)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				break;

			selectedQueueIndex++;
		}

		return selectedQueueIndex;
	}

}
