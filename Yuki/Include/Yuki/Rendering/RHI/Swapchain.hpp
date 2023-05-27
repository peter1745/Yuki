#pragma once

#include "Yuki/Rendering/RenderAPI.hpp"

namespace Yuki {

	class Swapchain
	{
	public:
		virtual ~Swapchain() = default;

		virtual void Destroy() = 0;

		virtual void BeginRendering(CommandBuffer* InCmdBuffer) = 0;
		virtual void EndRendering(CommandBuffer* InCmdBuffer) = 0;

	};

}
