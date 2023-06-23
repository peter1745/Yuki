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

	VkImageLayout VulkanHelper::ImageLayoutToVkImageLayout(ImageLayout InLayout)
	{
		switch (InLayout)
		{
		case ImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
		case ImageLayout::Attachment: return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		case ImageLayout::ShaderReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case ImageLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		case ImageLayout::TransferDestination: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case ImageLayout::TransferSource: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		}

		YUKI_VERIFY(false);
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	ImageLayout VulkanHelper::VkImageLayoutToImageLayout(VkImageLayout InLayout)
	{
		switch (InLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED: return ImageLayout::Undefined;
		case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL: return ImageLayout::Attachment;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return ImageLayout::ShaderReadOnly;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return ImageLayout::Present;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return ImageLayout::TransferDestination;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return ImageLayout::TransferSource;
		}

		YUKI_VERIFY(false);
		return ImageLayout::Undefined;
	}

}
