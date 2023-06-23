#pragma once

#include "VulkanInclude.hpp"

namespace Yuki {

	struct VulkanPipeline
	{
		VkPipelineLayout Layout = VK_NULL_HANDLE;
		VkPipeline Pipeline = VK_NULL_HANDLE;
	};

}
