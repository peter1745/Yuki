#include "Rendering/GraphicsPipelineBuilder.hpp"

#include "Vulkan/VulkanGraphicsPipelineBuilder.hpp"

namespace Yuki {

	Unique<GraphicsPipelineBuilder> GraphicsPipelineBuilder::New(RenderAPI InRenderAPI, RenderContext* InContext)
	{
		switch (InRenderAPI)
		{
		case RenderAPI::None: return nullptr;
		case RenderAPI::Vulkan: return Unique<VulkanGraphicsPipelineBuilder>::Create(InContext);
		}

		YUKI_VERIFY(false);
		return nullptr;
	}

}
