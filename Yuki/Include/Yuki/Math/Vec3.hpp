#pragma once

namespace Yuki::Math {

	struct Vec3
	{
		float X, Y, Z;

		Vec3& operator+=(const Vec3& InOther);
		Vec3& operator-=(const Vec3& InOther);
		Vec3& operator*=(const Vec3& InOther);
		Vec3& operator/=(const Vec3& InOther);

		Vec3 operator+(const Vec3& InOther);
		Vec3 operator-(const Vec3& InOther);
		Vec3 operator*(const Vec3& InOther);
		Vec3 operator/(const Vec3& InOther);
	};

}
