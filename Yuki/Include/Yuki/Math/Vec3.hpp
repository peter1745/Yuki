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

		constexpr Vec3()
			: X(0.0f), Y(0.0f), Z(0.0f)
		{}

		constexpr Vec3(float InX, float InY, float InZ)
			: X(InX), Y(InY), Z(InZ)
		{}

		constexpr Vec3(std::array<float, 3> InValues)
			: Values(InValues)
		{}

		float Dot(const Vec3& InOther) const;
		float Length() const;

		void Normalize();
		Vec3 Normalized() const;

		Vec3 Cross(const Vec3& InOther) const;

		Vec3& operator+=(const Vec3& InOther);
		Vec3& operator-=(const Vec3& InOther);
		Vec3& operator*=(const Vec3& InOther);
		Vec3& operator/=(const Vec3& InOther);

		Vec3& operator+=(const float InScalar);
		Vec3& operator-=(const float InScalar);
		Vec3& operator*=(const float InScalar);
		Vec3& operator/=(const float InScalar);

		Vec3 operator+(const Vec3& InOther) const;
		Vec3 operator-(const Vec3& InOther) const;
		Vec3 operator*(const Vec3& InOther) const;
		Vec3 operator/(const Vec3& InOther) const;

		Vec3 operator+(const float InScalar) const;
		Vec3 operator-(const float InScalar) const;
		Vec3 operator*(const float InScalar) const;
		Vec3 operator/(const float InScalar) const;

		float& operator[](size_t InIndex) { return Values[InIndex]; }
		const float& operator[](size_t InIndex) const { return Values[InIndex]; }

	};

}
