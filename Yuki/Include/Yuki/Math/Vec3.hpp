#pragma once

#include <array>

namespace Yuki::Math {

	struct Vec3
	{
		union
		{
			std::array<float, 3> Values;
			struct { float X, Y, Z; };
		};

		Vec3& operator+=(const Vec3& InOther);
		Vec3& operator-=(const Vec3& InOther);
		Vec3& operator*=(const Vec3& InOther);
		Vec3& operator/=(const Vec3& InOther);

		Vec3 operator+(const Vec3& InOther) const;
		Vec3 operator-(const Vec3& InOther) const;
		Vec3 operator*(const Vec3& InOther) const;
		Vec3 operator/(const Vec3& InOther) const;

		float& operator[](size_t InIndex) { return Values[InIndex]; }
		const float& operator[](size_t InIndex) const { return Values[InIndex]; }

	};

}
