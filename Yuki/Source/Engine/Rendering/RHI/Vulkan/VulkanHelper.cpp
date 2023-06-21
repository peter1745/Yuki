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

	void VulkanHelper::TransitionImage(VkCommandBuffer InCommandBuffer, VkImage InImage, VkPipelineStageFlags2 InSrcStage, VkAccessFlags2 InSrcAccess, VkImageLayout InSrcLayout, VkPipelineStageFlags2 InDstStage, VkAccessFlags2 InDstAccess, VkImageLayout InDstLayout, VkImageAspectFlags InAspectFlags)
	{
		if (InSrcLayout == InDstLayout)
			return;

		VkImageMemoryBarrier2 barrier =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = InSrcStage,
			.srcAccessMask = InSrcAccess,
			.dstStageMask = InDstStage,
			.dstAccessMask = InDstAccess,
			.oldLayout = InSrcLayout,
			.newLayout = InDstLayout,
			.image = InImage,
			.subresourceRange = {
				.aspectMask = InAspectFlags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};

		VkDependencyInfo dependencyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier,
		};

		vkCmdPipelineBarrier2(InCommandBuffer, &dependencyInfo);
	}

	VkFormat VulkanHelper::ImageFormatToVkFormat(ImageFormat InFormat)
	{
		switch (InFormat)
		{
		case ImageFormat::None: return VK_FORMAT_UNDEFINED;
		case ImageFormat::RGBA8UNorm: return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::BGRA8UNorm: return VK_FORMAT_B8G8R8A8_UNORM;
		case ImageFormat::Depth32SFloat: return VK_FORMAT_D32_SFLOAT; // TODO(Peter): VK_FORMAT_D32_SFLOAT for AMD
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
		case VK_FORMAT_D32_SFLOAT: return ImageFormat::Depth32SFloat;
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
		case BufferType::StorageBuffer:
			result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
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
		case ImageLayout::ShaderReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case ImageLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		YUKI_VERIFY(false);
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	VkFlags VulkanHelper::ImageUsageToVkFlags(ImageUsage InUsage)
	{
		VkFlags result = VK_IMAGE_LAYOUT_UNDEFINED;

		if (InUsage & ImageUsage::ColorAttachment) result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (InUsage & ImageUsage::DepthAttachment) result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (InUsage & ImageUsage::Sampled) result |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (InUsage & ImageUsage::TransferDestination) result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (InUsage & ImageUsage::TransferSource) result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		return result;
	}

	VkDescriptorType VulkanHelper::DescriptorTypeToVkDescriptorType(DescriptorType InType)
	{
		switch (InType)
		{
		case DescriptorType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case DescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}

		YUKI_VERIFY(false);
		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}

}
