#include "VulkanImage.hpp"
#include "VulkanRenderDevice.hpp"

namespace Yuki::RHI {

	ImageRH VulkanRenderDevice::ImageCreate(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) { return {}; }
	void VulkanRenderDevice::ImageDestroy(ImageRH InImage) {}

	ImageViewRH VulkanRenderDevice::ImageViewCreate(ImageRH InImage)
	{
		auto[Handle, ImageView] = m_ImageViews.Acquire();
		const auto& Image = m_Images[InImage];

		ImageView.Image = InImage;

		VkImageViewCreateInfo ImageViewInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = Image.Handle,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = Image.Format,
			.subresourceRange = {
				.aspectMask = Image.AspectMask,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
		};
		YUKI_VERIFY(vkCreateImageView(m_Device, &ImageViewInfo, nullptr, &ImageView.Handle) == VK_SUCCESS);

		return Handle;
	}

	void VulkanRenderDevice::ImageViewDestroy(ImageViewRH InImageView)
	{
		auto& ImageView = m_ImageViews[InImageView];
		vkDestroyImageView(m_Device, ImageView.Handle, nullptr);
		m_ImageViews.Return(InImageView);
	}

}
