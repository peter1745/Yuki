#pragma once

namespace Yuki {

	enum class ImageFormat
	{
		None = -1,

		RGBA8UNorm,
		BGRA8UNorm,
		D24UNormS8UInt
	};

	static constexpr bool IsDepthFormat(ImageFormat InFormat) { return InFormat == ImageFormat::D24UNormS8UInt; }

}
