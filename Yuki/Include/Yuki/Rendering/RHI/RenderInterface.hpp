#pragma once

#include "Rendering/RHI/CommandBuffer.hpp"
#include "Rendering/RHI/RenderTarget.hpp"

namespace Yuki {

	class RenderInterface
	{
	public:
		virtual ~RenderInterface() = default;

		virtual void BeginRendering(CommandBuffer InCmdBuffer, RenderTarget* InRenderTarget) = 0;
		virtual void EndRendering(CommandBuffer InCmdBuffer) = 0;

	};

}
