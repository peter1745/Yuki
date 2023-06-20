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
		case ImageFormat::None: return VK_FORMAT_UNDEFINED;
		case ImageFormat::RGBA8UNorm: return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::BGRA8UNorm: return VK_FORMAT_B8G8R8A8_UNORM;
		case ImageFormat::Depth24UNorm: return VK_FORMAT_X8_D24_UNORM_PACK32; // TODO(Peter): VK_FORMAT_D32_SFLOAT for AMD
		}

		YUKI_VERIFY(false);
		return VK_FORMAT_UNDEFINED;
	}

	ImageFormat VulkanHelper::VkFormatToImageFormat(VkFormat InFormat)
	{
		switch (InFormat)
		{
		case VK_FORMAT_R8G8B8A8_UNORM: return ImageFormat::RGBA8UNorm;
		case VK_FORMAT_B8G8R8A8_UNORM: return ImageFormat::BGRA8UNorm;
		case VK_FORMAT_X8_D24_UNORM_PACK32: return ImageFormat::Depth24UNorm;
		default:
			break;
		}

		return ImageFormat::None;
	}

	VkBufferUsageFlags VulkanHelper::BufferTypeToVkUsageFlags(BufferType InType)
	{
		VkBufferUsageFlags result = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		switch (InType)
		{
		case BufferType::VertexBuffer:
			result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			break;
		case BufferType::IndexBuffer:
			result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			break;
		case BufferType::StagingBuffer:
			break;
		}

		return result;
	}

	VkImageLayout VulkanHelper::ImageLayoutToVkImageLayout(ImageLayout InLayout)
	{
		switch (InLayout)
		{
		case ImageLayout::ColorAttachment: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case ImageLayout::DepthAttachment: return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		case ImageLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		YUKI_VERIFY(false);
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

}
