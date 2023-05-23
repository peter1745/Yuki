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

	VkFormat VulkanHelper::ImageFormatToVkFormat(ImageFormat InFormat)
	{
		switch (InFormat)
		{
		case ImageFormat::RGBA8UNorm: return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::BGRA8UNorm: return VK_FORMAT_B8G8R8A8_UNORM;
		case ImageFormat::D24UNormS8UInt: return VK_FORMAT_D24_UNORM_S8_UINT;
		}

		YUKI_VERIFY(false);
		return VK_FORMAT_UNDEFINED;
	}

}
