#pragma once

#include "Vec4.hpp"

namespace Yuki::Math {

	struct Quat
	{
		Vec4 Value;

		explicit Quat(float InAngle, const Vec3& InAxis);
	};

}
