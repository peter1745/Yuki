#include "Math/Vec2.hpp"

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

	Vec2& Vec2::operator+=(const float InOther)
	{
		X += InOther;
		Y += InOther;
		return *this;
	}

	Vec2& Vec2::operator-=(const float InOther)
	{
		X -= InOther;
		Y -= InOther;
		return *this;
	}

	Vec2& Vec2::operator*=(const float InOther)
	{
		X *= InOther;
		Y *= InOther;
		return *this;
	}

	Vec2& Vec2::operator/=(const float InOther)
	{
		X /= InOther;
		Y /= InOther;
		return *this;
	}

	Vec2 Vec2::operator+(const Vec2& InOther) const
	{
		return { X + InOther.X, Y + InOther.Y };
	}

	Vec2 Vec2::operator-(const Vec2& InOther) const
	{
		return { X - InOther.X, Y - InOther.Y };
	}

	Vec2 Vec2::operator*(const Vec2& InOther) const
	{
		return { X * InOther.X, Y * InOther.Y };
	}

	Vec2 Vec2::operator/(const Vec2& InOther) const
	{
		return { X / InOther.X, Y / InOther.Y };
	}

	Vec2 Vec2::operator+(const float InOther) const
	{
		return { X + InOther, Y + InOther };
	}

	Vec2 Vec2::operator-(const float InOther) const
	{
		return { X - InOther, Y - InOther };
	}

	Vec2 Vec2::operator*(const float InOther) const
	{
		return { X * InOther, Y * InOther };
	}

	Vec2 Vec2::operator/(const float InOther) const
	{
		return { X / InOther, Y / InOther };
	}

}
