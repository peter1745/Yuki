#pragma once

#include "Vec4.hpp"

namespace Yuki::Math {

	struct Quat
	{
		Vec4 Value;

		Quat(const Vec4& InValue)
			: Value(InValue) {}
		Quat(float InAngle, const Vec3& InAxis);
	};

}
