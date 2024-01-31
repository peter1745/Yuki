#include "VulkanRHI.hpp"

namespace Yuki {

	ImageView ImageView::Create(RHIContext context, Image image)
	{
		auto* impl = new Impl();
		impl->Context = context;
		impl->Source = image;

		VkImageViewCreateInfo viewInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image->Resource,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = image->Format,
			.subresourceRange = {
				.aspectMask = image->AspectFlags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		Vulkan::CheckResult(vkCreateImageView(context->Device, &viewInfo, nullptr, &impl->Resource));

		return { impl };
	}

	void ImageView::Destroy()
	{
		vkDestroyImageView(m_Impl->Context->Device, m_Impl->Resource, nullptr);
		delete m_Impl;
	}

	void CommandList::TransitionImage(Image image, ImageLayout layout) const
	{
		auto newLayout = ImageLayoutToVkImageLayout(layout);

		if (image->Layout == newLayout)
		{
			return;
		}

		image->OldLayout = image->Layout;
		image->Layout = newLayout;

		VkImageMemoryBarrier2 imageBarrier =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			.dstAccessMask = 0,
			.oldLayout = image->OldLayout,
			.newLayout = image->Layout,
			.image = image->Resource,
			.subresourceRange = {
				.aspectMask = image->AspectFlags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		VkDependencyInfo dependencyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &imageBarrier,
		};

		vkCmdPipelineBarrier2(m_Impl->Resource, &dependencyInfo);
	}

}
