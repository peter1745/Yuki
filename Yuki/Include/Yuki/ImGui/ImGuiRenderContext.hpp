#pragma once

#include "Yuki/Rendering/RHI.hpp"

namespace Yuki {

	class RenderContext;

	class ImGuiRenderContext
	{
	public:
		virtual ~ImGuiRenderContext() = default;

		virtual void NewFrame(CommandListHandle InCommandList) const = 0;
		virtual void EndFrame(CommandListHandle InCommandList) = 0;

	public:
		static Unique<ImGuiRenderContext> New(SwapchainHandle InSwapchain, RenderContext* InContext);
	};

}
