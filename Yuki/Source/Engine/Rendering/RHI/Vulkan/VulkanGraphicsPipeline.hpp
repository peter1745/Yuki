#pragma once

#include "Rendering/RHI/GraphicsPipeline.hpp"

#include "VulkanInclude.hpp"

namespace Yuki {

	struct VulkanGraphicsPipeline : public GraphicsPipeline
	{
		VulkanGraphicsPipeline() = default;

		VkPipeline Pipeline;
		VkPipelineLayout Layout;
	};

}
