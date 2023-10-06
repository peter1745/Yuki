#pragma once

#include "Engine/RHI/RenderHandles.hpp"

#include "VulkanInclude.hpp"

namespace Yuki::RHI {

	struct VulkanImage
	{
		VkImage Handle = VK_NULL_HANDLE;
		uint32_t Width = 0;
		uint32_t Height = 0;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkImageAspectFlags AspectMask = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	struct VulkanImageView
	{
		VkImageView Handle = VK_NULL_HANDLE;
		ImageRH Image{ -1 };
	};

	inline VkImageLayout ImageLayoutToVkImageLayout(ImageLayout InLayout)
	{
		switch (InLayout)
		{
		case ImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
		case ImageLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
		case ImageLayout::Attachment: return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		case ImageLayout::ShaderReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case ImageLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		case ImageLayout::TransferDest: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case ImageLayout::TransferSource: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		default: return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}

}
