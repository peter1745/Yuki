#pragma once

#include "Rendering/RHI/CommandBuffer.hpp"

namespace Yuki {

	class RenderInterface
	{
	public:
		virtual ~RenderInterface() = default;

		virtual void BeginRendering(CommandBuffer InCmdBuffer) = 0;
		virtual void EndRendering(CommandBuffer InCmdBuffer) = 0;

	};

}
