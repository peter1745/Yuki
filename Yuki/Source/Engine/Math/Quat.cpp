#include "Math/Quat.hpp"

namespace Yuki::Math {

	Quat::Quat(float InAngle, const Vec3& InAxis)
	{
		float halfAngle = InAngle * 0.5f;
		float halfAngleSin = std::sinf(halfAngle);

		Value = {
			InAxis.X * halfAngleSin,
			InAxis.Y * halfAngleSin,
			InAxis.Z * halfAngleSin,
			std::cosf(halfAngle)
		};
	}

}
