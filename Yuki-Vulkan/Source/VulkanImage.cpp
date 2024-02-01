#include "VulkanRHI.hpp"

namespace Yuki {

	Image Image::Create(RHIContext context, const ImageConfig& config)
	{
		auto* impl = new Impl();
		impl->Context = context;
		impl->Width = config.Width;
		impl->Height = config.Height;
		impl->Format = ImageFormatToVkFormat(config.Format);
		impl->OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		impl->Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		impl->AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageCreateInfo imageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = impl->Format,
			.extent = {
				.width = static_cast<uint32_t>(impl->Width),
				.height = static_cast<uint32_t>(impl->Height),
				.depth = 1
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = ImageUsageToVkImageUsage(config.Usage),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		impl->Allocation = context->Allocator.CreateImage(imageInfo);

		if (config.CreateDefaultView)
		{
			impl->DefaultView = ImageView::Create(context, { impl });
		}

		return { impl };
	}

	void Image::Destroy()
	{
		if (m_Impl->DefaultView)
		{
			m_Impl->DefaultView.Destroy();
		}

		m_Impl->Context->Allocator.DestroyImage(m_Impl->Allocation);
		delete m_Impl;
	}

	ImageView Image::GetDefaultView() const { return m_Impl->DefaultView; }

	ImageView ImageView::Create(RHIContext context, Image image)
	{
		auto* impl = new Impl();
		impl->Context = context;
		impl->Source = image;

		VkImageViewCreateInfo viewInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image->Allocation.Resource,
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

}
