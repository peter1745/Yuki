#include "Vec2.hpp"

namespace Yuki::Math {

	Vec2& Vec2::operator+=(const Vec2& InOther)
	{
		X += InOther.X;
		Y += InOther.Y;
		return *this;
	}

	Vec2& Vec2::operator-=(const Vec2& InOther)
	{
		X -= InOther.X;
		Y -= InOther.Y;
		return *this;
	}

	Vec2& Vec2::operator*=(const Vec2& InOther)
	{
		X *= InOther.X;
		Y *= InOther.Y;
		return *this;
	}

	Vec2& Vec2::operator/=(const Vec2& InOther)
	{
		X /= InOther.X;
		Y /= InOther.Y;
		return *this;
	}

	Vec2 Vec2::operator+(const Vec2& InOther)
	{
		return { X + InOther.X, Y + InOther.Y };
	}

	Vec2 Vec2::operator-(const Vec2& InOther)
	{
		return { X - InOther.X, Y - InOther.Y };
	}

	Vec2 Vec2::operator*(const Vec2& InOther)
	{
		return { X * InOther.X, Y * InOther.Y };
	}

	Vec2 Vec2::operator/(const Vec2& InOther)
	{
		return { X / InOther.X, Y / InOther.Y };
	}

}
