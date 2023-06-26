#include "Rendering/RenderContext.hpp"

#include "Vulkan/VulkanRenderContext.hpp"

namespace Yuki {

	Unique<RenderContext> RenderContext::New(RenderAPI InAPI)
	{
		switch (InAPI)
		{
		case RenderAPI::None: return nullptr;
		case RenderAPI::Vulkan: return new VulkanRenderContext();
		}

		YUKI_VERIFY(false);
		return nullptr;
	}

}
