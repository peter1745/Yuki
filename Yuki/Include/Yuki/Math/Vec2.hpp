#pragma once

namespace Yuki::Math {

	struct Vec2
	{
		float X, Y;

		Vec2& operator+=(const Vec2& InOther);
		Vec2& operator-=(const Vec2& InOther);
		Vec2& operator*=(const Vec2& InOther);
		Vec2& operator/=(const Vec2& InOther);

		Vec2 operator+(const Vec2& InOther);
		Vec2 operator-(const Vec2& InOther);
		Vec2 operator*(const Vec2& InOther);
		Vec2 operator/(const Vec2& InOther);
	};

}
