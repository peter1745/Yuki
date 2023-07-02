#include "Math/Quat.hpp"
#include "Math/Math.hpp"

namespace Yuki::Math {

	Quat::Quat()
			: Value(0.0f, 0.0f, 0.0f, 1.0f) {}

	Quat::Quat(const Vec3& InEulerAngles)
	{
		float cosX = Math::Cos(InEulerAngles.X * 0.5f);
		float sinX = Math::Sin(InEulerAngles.X * 0.5f);
		float cosY = Math::Cos(InEulerAngles.Y * 0.5f);
		float sinY = Math::Sin(InEulerAngles.Y * 0.5f);
		float cosZ = Math::Cos(InEulerAngles.Z * 0.5f);
		float sinZ = Math::Sin(InEulerAngles.Z * 0.5f);

		Value = {
			sinX * cosY * cosZ - cosX * sinY * sinZ,
			cosX * sinY * cosZ + sinX * cosY * sinZ,
			cosX * cosY * sinZ - sinX * sinY * cosZ,
			cosX * cosY * cosZ + sinX * sinY * sinZ,
		};
	}

	Quat::Quat(const Vec4& InValue)
			: Value(InValue) {}

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

	Vec3 Quat::EulerAngles() const
	{
		Vec3 result;
		
		float x0 = 2.0f * (Value.W * Value.X + Value.Y * Value.Z);
		float x1 = 1.0f - 2.0f * (Value.X * Value.X + Value.Y * Value.Y);
		result.X = Math::Atan2(x0, x1);

		float y0 = Math::Sqrt(1.0f + 2.0f * (Value.W * Value.Y - Value.X * Value.Z));
		float y1 = Math::Sqrt(1.0f - 2.0f * (Value.W * Value.Y - Value.X * Value.Z));
		result.Y = 2.0f * Math::Atan2(y0, y1) - Math::PI<float>() * 0.5f;

		float z0 = 2.0f * (Value.W * Value.Z + Value.X * Value.Y);
		float z1 = 1.0f - 2.0f * (Value.Y * Value.Y + Value.Z * Value.Z);
		result.Z = Math::Atan2(z0, z1);

		return result;
	}

}
