#pragma once

#include "Vec4.hpp"

namespace Yuki::Math {

	struct Quat
	{
		Vec4 Value;

		Quat();
		Quat(const Vec3& InEulerAngles);
		Quat(const Vec4& InValue);
		Quat(float InAngle, const Vec3& InAxis);

		Vec3 EulerAngles() const;
	};

}
