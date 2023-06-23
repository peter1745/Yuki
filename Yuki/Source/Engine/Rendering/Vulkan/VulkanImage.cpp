#include "VulkanImage.hpp"
#include "VulkanRenderContext.hpp"

namespace Yuki {

	static VkImageLayout ImageLayoutToVkImageLayout(ImageLayout InLayout)
	{
		switch (InLayout)
		{
		case ImageLayout::Attachment: return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		case ImageLayout::ShaderReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case ImageLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		YUKI_VERIFY(false);
		return VK_IMAGE_LAYOUT_UNDEFINED;
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
		vkDestroyImageView(m_LogicalDevice, imageView.ImageView, nullptr);
		m_ImageViews.Return(InImageView);
	}

	void VulkanRenderContext::ImageTransition(CommandList InCommandList, Image InImage, ImageLayout InNewLayout)
	{
		auto& image = m_Images.Get(InImage);
		auto& commandList = m_CommandLists.Get(InCommandList);
		auto newLayout = ImageLayoutToVkImageLayout(InNewLayout);

		// TODO(Peter): Mips / Array Levels

		VkImageMemoryBarrier2 barrier =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = image.PipelineStage,
			.srcAccessMask = image.AccessFlags,
			.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			.dstAccessMask = 0,
			.oldLayout = image.Layout,
			.newLayout = newLayout,
			.image = image.Image,
			.subresourceRange = {
				.aspectMask = image.AspectFlags,
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

		vkCmdPipelineBarrier2(commandList.CommandBuffer, &dependencyInfo);

		image.Layout = newLayout;
		image.PipelineStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		image.AccessFlags = 0;
	}

}
