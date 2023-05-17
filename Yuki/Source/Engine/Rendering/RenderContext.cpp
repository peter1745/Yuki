#include "Rendering/RenderContext.hpp"

#include "Vulkan/VulkanRenderContext.hpp"

namespace Yuki {

	Unique<RenderContext> RenderContext::New(RenderAPI InRenderAPI)
	{
		switch (InRenderAPI)
		{
		case RenderAPI::None: return nullptr;
		case RenderAPI::Vulkan: return Unique<VulkanRenderContext>::Create();
		}

		return nullptr;
	}

}
