#include "Vec3.hpp"

namespace Yuki::Math {

	Vec3& Vec3::operator+=(const Vec3& InOther)
	{
		X += InOther.X;
		Y += InOther.Y;
		Z += InOther.Z;
		return *this;
	}

	Vec3& Vec3::operator-=(const Vec3& InOther)
	{
		X -= InOther.X;
		Y -= InOther.Y;
		Z -= InOther.Z;
		return *this;
	}

	Vec3& Vec3::operator*=(const Vec3& InOther)
	{
		X *= InOther.X;
		Y *= InOther.Y;
		Z *= InOther.Z;
		return *this;
	}

	Vec3& Vec3::operator/=(const Vec3& InOther)
	{
		X /= InOther.X;
		Y /= InOther.Y;
		Z /= InOther.Z;
		return *this;
	}

	Vec3 Vec3::operator+(const Vec3& InOther)
	{
		return { X + InOther.X, Y + InOther.Y, Z + InOther.Z };
	}

	Vec3 Vec3::operator-(const Vec3& InOther)
	{
		return { X - InOther.X, Y - InOther.Y, Z - InOther.Z };
	}

	Vec3 Vec3::operator*(const Vec3& InOther)
	{
		return { X * InOther.X, Y * InOther.Y, Z * InOther.Z };
	}

	Vec3 Vec3::operator/(const Vec3& InOther)
	{
		return { X / InOther.X, Y / InOther.Y, Z / InOther.Z };
	}

}
