#include "VulkanImage2D.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	VulkanImage2D::VulkanImage2D(VulkanRenderContext* InContext, uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage)
		: m_Context(InContext), m_Width(InWidth), m_Height(InHeight), m_Format(InFormat)
	{
		uint32_t queueFamilyIndex = static_cast<VulkanQueue*>(InContext->GetGraphicsQueue())->GetFamilyIndex();

		VkImageCreateInfo imageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VulkanHelper::ImageFormatToVkFormat(InFormat),
			.extent = {
				.width = InWidth,
				.height = InHeight,
				.depth = 1},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VulkanHelper::ImageUsageToVkFlags(InUsage),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &queueFamilyIndex,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};
		m_Image = InContext->GetAllocator().CreateImage(&imageInfo, &m_Allocation);

		m_DefaultImageView = Unique<ImageView2D>(new VulkanImageView2D(InContext, this));
	}

	VulkanImage2D::VulkanImage2D(VulkanRenderContext* InContext, uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, VkImage InExistingImage)
		: m_Context(InContext), m_Width(InWidth), m_Height(InHeight), m_Format(InFormat), m_Image(InExistingImage)
	{
	}

	VulkanImage2D::~VulkanImage2D()
	{
		m_DefaultImageView.Reset();

		m_Context->GetAllocator().DestroyImage(m_Image, m_Allocation);
	}

	VulkanImageView2D::VulkanImageView2D(VulkanRenderContext* InContext, VulkanImage2D* InImage)
		: m_Context(InContext), m_Image(InImage)
	{
		VkImageAspectFlags aspectMask = IsDepthFormat(InImage->GetImageFormat()) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageViewCreateInfo imageViewInfo =
		{
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

		vkCreateImageView(InContext->GetDevice(), &imageViewInfo, nullptr, &m_ImageView);
	}

	VulkanImageView2D::~VulkanImageView2D()
	{
		vkDestroyImageView(m_Context->GetDevice(), m_ImageView, nullptr);
	}

}
