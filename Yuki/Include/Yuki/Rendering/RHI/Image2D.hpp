#pragma once

#include "Yuki/Rendering/ImageFormat.hpp"

namespace Yuki {

	class Image2D
	{
	public:
		virtual ~Image2D() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual ImageFormat GetImageFormat() const = 0;
	};

	class ImageView2D
	{
	public:
		virtual ~ImageView2D() = default;

		virtual Image2D* GetImage() const = 0;
	};

}
