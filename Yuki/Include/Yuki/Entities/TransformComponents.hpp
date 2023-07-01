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
		float Value;
	};

	struct LocalTransform
	{
		Math::Mat4 Value;
	};

	struct GPULocalTransform
	{
		uint32_t BufferIndex;
	};

}
