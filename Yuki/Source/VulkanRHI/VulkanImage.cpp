#include "VulkanRHI.hpp"

namespace Yuki::RHI {

	//Image Image::Create(Context InContext, uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) { return {}; }
	//void VulkanRenderDevice::ImageDestroy(ImageRH InImage) {}

	ImageView ImageView::Create(Context InContext, ImageRH InImage)
	{
		auto ImageView = new Impl();
		ImageView->Ctx = InContext;
		ImageView->Image = InImage;

		VkImageViewCreateInfo ImageViewInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = InImage->Handle,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = InImage->Format,
			.subresourceRange = {
				.aspectMask = InImage->AspectMask,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
		};
		YUKI_VERIFY(vkCreateImageView(InContext->Device, &ImageViewInfo, nullptr, &ImageView->Handle) == VK_SUCCESS);

		return { ImageView };
	}

	void ImageView::Destroy()
	{
		vkDestroyImageView(m_Impl->Ctx->Device, m_Impl->Handle, nullptr);
		delete m_Impl;
	}

}
