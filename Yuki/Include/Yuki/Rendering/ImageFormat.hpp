#pragma once

// I fucking hate Xlib (They define None)
#ifdef None
	#undef None
#endif

namespace Yuki {

	enum class ImageFormat
	{
		None = -1,

		RGBA8UNorm,
		BGRA8UNorm,
		Depth32SFloat
	};

	static constexpr bool IsDepthFormat(ImageFormat InFormat) { return InFormat == ImageFormat::Depth32SFloat; }

}
