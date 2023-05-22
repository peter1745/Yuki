#pragma once

#include "Rendering/RHI/GraphicsPipeline.hpp"

#include "Vulkan.hpp"

namespace Yuki {

	struct VulkanGraphicsPipeline : public GraphicsPipeline
	{
		VulkanGraphicsPipeline() = default;

		VkPipeline Pipeline;
		VkPipelineLayout Layout;
	};

}
