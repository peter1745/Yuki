#pragma once

#include "Yuki/Math/Vec3.hpp"
#include "Yuki/Math/Quat.hpp"
#include "Yuki/Math/Mat4.hpp"

namespace Yuki::Entities {

	struct Translation
	{
		Math::Vec3 Value;
	};

	struct Rotation
	{
		Math::Quat Value;
	};

	struct Scale
	{
		Math::Vec3 Value;
	};

	struct LocalTransform
	{
		Math::Mat4 Value;
	};

	struct GPUTransform
	{
		uint32_t BufferIndex;
	};

	struct Name
	{
		std::string_view Value;
	};

}
