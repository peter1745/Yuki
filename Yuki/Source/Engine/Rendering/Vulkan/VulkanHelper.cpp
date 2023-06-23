#include "VulkanHelper.hpp"

namespace Yuki {

	VkFormat VulkanHelper::ImageFormatToVkFormat(ImageFormat InFormat)
	{
		switch (InFormat)
		{
		case ImageFormat::None: return VK_FORMAT_UNDEFINED;
		case ImageFormat::RGBA8UNorm: return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::BGRA8UNorm: return VK_FORMAT_B8G8R8A8_UNORM;
		case ImageFormat::Depth32SFloat: return VK_FORMAT_D32_SFLOAT;
		}

		YUKI_VERIFY(false);
		return VK_FORMAT_UNDEFINED;
	}

}
