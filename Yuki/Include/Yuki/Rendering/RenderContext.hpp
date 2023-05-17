#pragma once

#include "../Memory/Unique.hpp"

#include "RenderAPI.hpp"
#include "Swapchain.hpp"

namespace Yuki {

	class GenericWindow;

	class RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void Initialize() = 0;
		virtual void Destroy() = 0;

		virtual Unique<Swapchain> CreateSwapchain(GenericWindow* InWindow) const = 0;

		static Unique<RenderContext> New(RenderAPI InRenderAPI);
	};

}
