#pragma once

#include "Yuki/Rendering/RenderAPI.hpp"

namespace Yuki {

	class Image2D;
	class ImageView2D;

	class Swapchain
	{
	public:
		virtual ~Swapchain() = default;

		virtual Image2D* GetCurrentImage() const = 0;
		virtual ImageView2D* GetCurrentImageView() const = 0;

		virtual void Destroy() = 0;

	};

}
