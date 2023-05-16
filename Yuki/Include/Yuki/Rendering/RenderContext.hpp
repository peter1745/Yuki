#pragma once

#include "../Core/GenericWindow.hpp"
#include "../Memory/Unique.hpp"

namespace Yuki {

	class RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void Initialize() = 0;
		virtual void Destroy() = 0;

		 static Unique<RenderContext> New(GenericWindow* InWindow);
	};

}
