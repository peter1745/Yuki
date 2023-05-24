#pragma once

#include "Rendering/RHI/RenderInterface.hpp"

namespace Yuki {

	class VulkanRenderInterface : public RenderInterface
	{
	public:
		void BeginRendering(CommandBuffer InCmdBuffer, RenderTarget* InRenderTarget) override;
		void EndRendering(CommandBuffer InCmdBuffer) override;

	};

}
