#include "VulkanImage2D.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	void VulkanImage2D::Transition(VkCommandBuffer InCommandBuffer, const VulkanImageTransition& InTransitionInfo)
	{
		VkImageAspectFlags aspectFlags = IsDepthFormat(m_Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageMemoryBarrier2 barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = m_CurrentPipelineStage,
			.srcAccessMask = m_CurrentAccessFlags,
			.dstStageMask = InTransitionInfo.DstPipelineStage,
			.dstAccessMask = InTransitionInfo.DstAccessFlags,
			.oldLayout = m_CurrentLayout,
			.newLayout = InTransitionInfo.DstImageLayout,
			.image = m_Image,
			.subresourceRange = {
			    .aspectMask = aspectFlags,
			    .baseMipLevel = 0,
			    .levelCount = 1,
			    .baseArrayLayer = 0,
			    .layerCount = 1,
			}
		};

		VkDependencyInfo dependencyInfo = {
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier,
		};

		vkCmdPipelineBarrier2(InCommandBuffer, &dependencyInfo);

		m_CurrentPipelineStage = InTransitionInfo.DstPipelineStage;
		m_CurrentAccessFlags = InTransitionInfo.DstAccessFlags;
		m_CurrentLayout = InTransitionInfo.DstImageLayout;
	}

	VulkanImage2D* VulkanImage2D::Create(VulkanRenderContext* InContext, uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat)
	{
		VulkanImage2D* image = new VulkanImage2D();
		image->m_Width = InWidth;
		image->m_Height = InHeight;
		image->m_Format = InFormat;

		uint32_t queueFamilyIndex = static_cast<VulkanQueue*>(InContext->GetGraphicsQueue())->GetFamilyIndex();

		VkFlags baseImageUsage = IsDepthFormat(InFormat) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		VkImageCreateInfo imageInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VulkanHelper::ImageFormatToVkFormat(InFormat),
			.extent = {
				.width = InWidth,
				.height = InHeight,
				.depth = 1
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = baseImageUsage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &queueFamilyIndex,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};
		image->m_Image = InContext->GetAllocator().CreateImage(&imageInfo, &image->m_Allocation);

		return image;
	}

	VulkanImage2D* VulkanImage2D::Create(VulkanRenderContext* InContext, uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, VkImage InExistingImage)
	{
		VulkanImage2D* image = new VulkanImage2D();
		image->m_Width = InWidth;
		image->m_Height = InHeight;
		image->m_Format = InFormat;
		image->m_Image = InExistingImage;
		return image;
	}

	void VulkanImage2D::Destroy(VulkanRenderContext* InContext, VulkanImage2D* InImage)
	{
		InContext->GetAllocator().DestroyImage(InImage->m_Image, InImage->m_Allocation);
		delete InImage;
	}

	VulkanImageView2D* VulkanImageView2D::Create(VulkanRenderContext* InContext, VulkanImage2D* InImage)
	{
		VulkanImageView2D* imageView = new VulkanImageView2D();
		imageView->m_Image = InImage;

		VkImageAspectFlags aspectMask = IsDepthFormat(InImage->GetImageFormat()) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageViewCreateInfo imageViewInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = InImage->GetVkImage(),
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VulkanHelper::ImageFormatToVkFormat(InImage->GetImageFormat()),
			.subresourceRange = {
			    .aspectMask = aspectMask,
			    .baseMipLevel = 0,
			    .levelCount = 1,
			    .baseArrayLayer = 0,
			    .layerCount = 1,
			},
		};

		vkCreateImageView(InContext->GetDevice(), &imageViewInfo, nullptr, &imageView->m_ImageView);
		return imageView;
	}

	void VulkanImageView2D::Destroy(VulkanRenderContext* InContext, VulkanImageView2D* InImageView)
	{
		vkDestroyImageView(InContext->GetDevice(), InImageView->m_ImageView, nullptr);
		delete InImageView;
	}

}
