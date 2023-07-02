#pragma once

#include "Yuki/Rendering/RHI.hpp"

#include <imgui/imgui.h>

namespace Yuki {

	class RenderContext;

	class ImGuiRenderContext
	{
	public:
		virtual ~ImGuiRenderContext() = default;

		virtual void NewFrame(CommandListHandle InCommandList) const = 0;
		virtual void EndFrame(CommandListHandle InCommandList) = 0;

		virtual void DrawImage(ImageHandle InImageHandle, ImVec2 InSize) = 0;
		virtual void RecreateImage(ImageHandle InImageHandle) = 0;

	public:
		static Unique<ImGuiRenderContext> New(SwapchainHandle InSwapchain, RenderContext* InContext);
	};

}
