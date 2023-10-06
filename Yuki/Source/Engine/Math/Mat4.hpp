#pragma once

#include "Vec3.hpp"
#include "Vec4.hpp"

namespace Yuki::Math {

	struct Mat4
	{
		Vec4 Columns[4];

		Mat4();

		static Mat4 Translation(const Vec3& InTranslation);

		static Mat4 PerspectiveInfReversedZ(FPType InFovY, FPType InAspectRatio, FPType InNearZ);
	};

}
