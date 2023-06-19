#pragma once

#include "RenderContext.hpp"
#include "Image2D.hpp"

namespace Yuki {

	struct RenderTarget
	{
		static constexpr size_t MaxColorAttachments = 8;

		uint32_t Width;
		uint32_t Height;
		std::array<ImageView2D*, MaxColorAttachments> ColorAttachments;
		ImageView2D* DepthAttachment = nullptr;
	};

}
