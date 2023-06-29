#include "ImGui/ImGuiRenderContext.hpp"
#include "Rendering/RenderContext.hpp"

#include "ImGuiVulkanRenderContext.hpp"

namespace Yuki {

	Unique<ImGuiRenderContext> ImGuiRenderContext::New(SwapchainHandle InSwapchain, RenderContext* InContext)
	{
		switch (InContext->GetRenderAPI())
		{
		case RenderAPI::None: return nullptr;
		case RenderAPI::Vulkan: return new ImGuiVulkanRenderContext(InSwapchain, InContext);
		}

		YUKI_VERIFY(false);
		return nullptr;
	}

}
