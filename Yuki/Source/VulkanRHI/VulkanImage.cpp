#include "VulkanRHI.hpp"

namespace Yuki::RHI {

	Image Image::Create(Context context, uint32_t width, uint32_t height, ImageFormat format, ImageUsage usage)
	{
		auto image = new Impl();
		image->Ctx = context;
		image->Width = width;
		image->Height = height;
		image->Format = Impl::ImageFormatToVkFormat(format);
		image->AspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageCreateInfo imageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = image->Format,
			.extent = {
				.width = image->Width,
				.height = image->Height,
				.depth = 1
			},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = Impl::ImageUsageToVkImageUsageFlags(usage),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		VmaAllocationCreateInfo allocInfo =
		{
			.usage = VMA_MEMORY_USAGE_AUTO,
			.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		};

		YUKI_VK_CHECK(vmaCreateImage(context->Allocator, &imageInfo, &allocInfo, &image->Handle, &image->Allocation, nullptr));

		image->DefaultView = ImageView::Create(context, { image });
		return { image };
	}

	uint32_t Image::GetWidth() const
	{
		return m_Impl->Width;
	}

	uint32_t Image::GetHeight() const
	{
		return m_Impl->Height;
	}

	void Image::Transition(ImageLayout layout) const
	{
		VkHostImageLayoutTransitionInfoEXT transitionInfo =
		{
			.sType = VK_STRUCTURE_TYPE_HOST_IMAGE_LAYOUT_TRANSITION_INFO_EXT,
			.pNext = nullptr,
			.image = m_Impl->Handle,
			.oldLayout = m_Impl->Layout,
			.newLayout = Impl::ImageLayoutToVkImageLayout(layout),
			.subresourceRange = {
				.aspectMask = m_Impl->AspectMask,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};
		vkTransitionImageLayoutEXT(m_Impl->Ctx->Device, 1, &transitionInfo);
	}

	void Image::SetData(const void* data) const
	{
		Transition(ImageLayout::General);

		VkMemoryToImageCopyEXT region =
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_TO_IMAGE_COPY_EXT,
			.pNext = nullptr,
			.pHostPointer = data,
			.imageSubresource = {
				.aspectMask = m_Impl->AspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = { 0, 0, 0},
			.imageExtent = { m_Impl->Width, m_Impl->Height, 1 },
		};

		VkCopyMemoryToImageInfoEXT copyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COPY_MEMORY_TO_IMAGE_INFO_EXT,
			.pNext = nullptr,
			.flags = 0,
			.dstImage = m_Impl->Handle,
			.dstImageLayout = VK_IMAGE_LAYOUT_GENERAL,
			.regionCount = 1,
			.pRegions = &region,
		};
		vkCopyMemoryToImageEXT(m_Impl->Ctx->Device, &copyInfo);

		m_Impl->OldLayout = m_Impl->Layout;
		m_Impl->Layout = VK_IMAGE_LAYOUT_GENERAL;
	}

	void Image::Destroy()
	{
		m_Impl->DefaultView.Destroy();
		vmaDestroyImage(m_Impl->Ctx->Allocator, m_Impl->Handle, m_Impl->Allocation);
		delete m_Impl;
	}

	ImageView Image::GetDefaultView() const { return m_Impl->DefaultView; }

	ImageView ImageView::Create(Context context, ImageRH image)
	{
		auto imageView = new Impl();
		imageView->Ctx = context;
		imageView->Image = image;

		VkImageViewCreateInfo imageViewInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = image->Handle,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = image->Format,
			.subresourceRange = {
				.aspectMask = image->AspectMask,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
		};
		YUKI_VK_CHECK(vkCreateImageView(context->Device, &imageViewInfo, nullptr, &imageView->Handle));

		return { imageView };
	}

	void ImageView::Destroy()
	{
		vkDestroyImageView(m_Impl->Ctx->Device, m_Impl->Handle, nullptr);
		delete m_Impl;
	}

	void CommandList::CopyImage(Image dest, Image src) const
	{
		VkImageCopy2 region =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2,
			.pNext = nullptr,
			.srcSubresource = {
				.aspectMask = src->AspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffset = { 0, 0, 0 },
			.dstSubresource = {
				.aspectMask = dest->AspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffset = { 0, 0, 0 },
			.extent = { src->Width, src->Height, 1 },
		};

		VkCopyImageInfo2 copyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
			.pNext = nullptr,
			.srcImage = src->Handle,
			.srcImageLayout = src->Layout,
			.dstImage = dest->Handle,
			.dstImageLayout = dest->Layout,
			.regionCount = 1,
			.pRegions = &region,
		};
		vkCmdCopyImage2(m_Impl->Handle, &copyInfo);
	}

	void CommandList::BlitImage(Image dest, Image src) const
	{
		VkImageBlit2 blitRegion =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
			.pNext = nullptr,
			.srcSubresource = {
				.aspectMask = src->AspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffsets = {
				{ 0, 0, 0 },
				{ Cast<int32_t>(src->Width), Cast<int32_t>(src->Height), 1 }
			},
			.dstSubresource = {
				.aspectMask = dest->AspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffsets = {
				{ 0, 0, 0 },
				{ Cast<int32_t>(dest->Width), Cast<int32_t>(dest->Height), 1 }
			}
		};

		VkBlitImageInfo2 blitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
			.pNext = nullptr,
			.srcImage = src->Handle,
			.srcImageLayout = src->Layout,
			.dstImage = dest->Handle,
			.dstImageLayout = dest->Layout,
			.regionCount = 1,
			.pRegions = &blitRegion,
			.filter = VK_FILTER_LINEAR,
		};

		vkCmdBlitImage2(m_Impl->Handle, &blitInfo);
	}

	void CommandList::ImageBarrier(RHI::ImageBarrier barrier)
	{
		YUKI_VERIFY(barrier.Images.Count() == barrier.Layouts.Count());

		DynamicArray<VkImageMemoryBarrier2> barriers(barrier.Images.Count(), { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 });

		for (size_t i = 0; i < barrier.Images.Count(); i++)
		{
			auto image = barrier.Images[i];
			auto newLayout = Image::Impl::ImageLayoutToVkImageLayout(barrier.Layouts[i]);

			image->OldLayout = image->Layout;

			barriers[i].srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			barriers[i].srcAccessMask = 0;
			barriers[i].dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			barriers[i].dstAccessMask = 0;
			barriers[i].oldLayout = image->OldLayout;
			barriers[i].newLayout = newLayout;
			barriers[i].image = image->Handle;
			barriers[i].subresourceRange =
			{
				.aspectMask = image->AspectMask,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			};

			image->Layout = newLayout;
		}

		VkDependencyInfo dependencyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.imageMemoryBarrierCount = Cast<uint32_t>(barriers.size()),
			.pImageMemoryBarriers = barriers.data(),
		};
		vkCmdPipelineBarrier2(m_Impl->Handle, &dependencyInfo);
	}

	Sampler Sampler::Create(Context context)
	{
		auto impl = new Impl();

		VkSamplerCreateInfo samplerInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		};
		YUKI_VK_CHECK(vkCreateSampler(context->Device, &samplerInfo, nullptr, &impl->Handle));

		return { impl };
	}

}
