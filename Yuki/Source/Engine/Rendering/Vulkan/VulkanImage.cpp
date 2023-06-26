#include "VulkanImage.hpp"
#include "VulkanRenderContext.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	static VkImageUsageFlags ImageUsageToVkImageUsageFlags(ImageUsage InUsage)
	{
		VkImageUsageFlags result = 0;

		if (InUsage & ImageUsage::ColorAttachment) result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (InUsage & ImageUsage::DepthAttachment) result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (InUsage & ImageUsage::Sampled) result |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (InUsage & ImageUsage::TransferDestination) result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (InUsage & ImageUsage::TransferDestination) result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		return result;
	}

	Image VulkanRenderContext::CreateImage(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage)
	{
		auto[handle, image] = m_Images.Acquire();

		auto& queue = m_Queues.Get(GetGraphicsQueue());

		VmaAllocationCreateInfo allocationInfo = { .usage = VMA_MEMORY_USAGE_AUTO };

		VkImageCreateInfo imageCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VulkanHelper::ImageFormatToVkFormat(InFormat),
			.extent = { InWidth, InHeight, 1 },
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = ImageUsageToVkImageUsageFlags(InUsage),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &queue.FamilyIndex,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		vmaCreateImage(m_Allocator, &imageCreateInfo, &allocationInfo, &image.Image, &image.Allocation, nullptr);

		image.Width = InWidth;
		image.Height = InHeight;
		image.Format = imageCreateInfo.format;
		image.AspectFlags = IsDepthFormat(InFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		image.Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		image.PipelineStage = VK_PIPELINE_STAGE_2_NONE;
		image.AccessFlags = VK_ACCESS_2_NONE;

		image.DefaultImageView = CreateImageView(handle);

		return handle;
	}

	void VulkanRenderContext::Destroy(Image InImage)
	{
		auto& image = m_Images.Get(InImage);
		Destroy(image.DefaultImageView);
		if (image.Allocation != nullptr)
			vmaDestroyImage(m_Allocator, image.Image, image.Allocation);
		m_Images.Return(InImage);
	}

	ImageView VulkanRenderContext::CreateImageView(Image InImage)
	{
		auto[handle, imageView] = m_ImageViews.Acquire();
		
		auto& image = m_Images.Get(InImage);
		imageView.SourceImage = InImage;

		VkImageViewCreateInfo imageViewCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = image.Image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = image.Format,
			.subresourceRange = {
				.aspectMask = image.AspectFlags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		vkCreateImageView(m_LogicalDevice, &imageViewCreateInfo, nullptr, &imageView.ImageView);
		return handle;
	}

	void VulkanRenderContext::Destroy(ImageView InImageView)
	{
		auto& imageView = m_ImageViews.Get(InImageView);

		if (imageView.ImageView == VK_NULL_HANDLE)
			return;

		vkDestroyImageView(m_LogicalDevice, imageView.ImageView, nullptr);
		imageView.ImageView = VK_NULL_HANDLE;
		m_ImageViews.Return(InImageView);
	}

}
