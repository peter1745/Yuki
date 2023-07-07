#pragma once

#include "Vec3.hpp"

namespace Yuki::Math {

	struct Vec4
	{
		union
		{
			std::array<float, 4> Values;
			struct { float X, Y, Z, W; };
			struct { float R, G, B, A; };
		};

		Vec4()
			: Values{0.0f, 0.0f, 0.0f, 0.0f} {}

		Vec4(const Vec3& InXYZ, float InW)
			: Values{InXYZ.X, InXYZ.Y, InXYZ.Z, InW}
		{}

		Vec4(std::array<float, 4> InValues)
			: Values(InValues) {}

		Vec4(float X, float Y, float Z, float W)
			: Values{X, Y, Z, W}
		{}

		Vec4& operator+=(const Vec4& InOther);
		Vec4& operator-=(const Vec4& InOther);
		Vec4& operator*=(const Vec4& InOther);
		Vec4& operator/=(const Vec4& InOther);

		Vec4& operator+=(const float InOther);
		Vec4& operator-=(const float InOther);
		Vec4& operator*=(const float InOther);
		Vec4& operator/=(const float InOther);

		Vec4 operator+(const Vec4& InOther) const;
		Vec4 operator-(const Vec4& InOther) const;
		Vec4 operator*(const Vec4& InOther) const;
		Vec4 operator/(const Vec4& InOther) const;

		Vec4 operator+(const float InOther) const;
		Vec4 operator-(const float InOther) const;
		Vec4 operator*(const float InOther) const;
		Vec4 operator/(const float InOther) const;

		float& operator[](size_t InIndex) { return Values[InIndex]; }
		const float& operator[](size_t InIndex) const { return Values[InIndex]; }

	};

}
