#pragma once

#include "VulkanInclude.hpp"
#include "Rendering/RHI.hpp"

#include <vma/vk_mem_alloc.h>

namespace Yuki {

	struct VulkanImage
	{
		VkImage Image = VK_NULL_HANDLE;
		uint32_t Width = 0;
		uint32_t Height = 0;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkImageUsageFlags Usage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
		VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_NONE;
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkPipelineStageFlags2 PipelineStage = VK_PIPELINE_STAGE_2_NONE;
		VkAccessFlags2 AccessFlags = VK_ACCESS_2_NONE;

		VmaAllocation Allocation = nullptr;

		ImageViewHandle DefaultImageView{};
	};

	struct VulkanImageView
	{
		ImageHandle SourceImage = {};
		VkImageView ImageView = VK_NULL_HANDLE;
	};

}
