#pragma once

#include "RenderContext.hpp"
#include "Image2D.hpp"

namespace Yuki {

	struct RenderTarget
	{
		static constexpr size_t MaxColorAttachments = 8;
		std::array<ImageView2D*, MaxColorAttachments> ColorAttachments;
		ImageView2D* DepthAttachment = nullptr;
	};

}
