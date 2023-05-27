#pragma once

#include "Yuki/Rendering/RHI/CommandBuffer.hpp"
#include "Yuki/Rendering/RHI/RenderTarget.hpp"
#include "Yuki/Rendering/RHI/GraphicsPipeline.hpp"

namespace Yuki {

	class RenderInterface
	{
	public:
		virtual ~RenderInterface() = default;

		virtual void BeginRendering(CommandBuffer* InCmdBuffer, RenderTarget* InRenderTarget) = 0;
		virtual void EndRendering(CommandBuffer* InCmdBuffer) = 0;

	};

}
