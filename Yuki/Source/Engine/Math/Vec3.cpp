#include "Math/Vec3.hpp"

namespace Yuki::Math {

	Vec3& Vec3::operator+=(const Vec3& InOther)
	{
		for (size_t i = 0; i < 3; i++)
			Values[i] += InOther[i];
		return *this;
	}

	Vec3& Vec3::operator-=(const Vec3& InOther)
	{
		for (size_t i = 0; i < 3; i++)
			Values[i] -= InOther[i];
		return *this;
	}

	Vec3& Vec3::operator*=(const Vec3& InOther)
	{
		for (size_t i = 0; i < 3; i++)
			Values[i] *= InOther[i];
		return *this;
	}

	Vec3& Vec3::operator/=(const Vec3& InOther)
	{
		for (size_t i = 0; i < 3; i++)
			Values[i] /= InOther[i];
		return *this;
	}

	Vec3 Vec3::operator+(const Vec3& InOther) const
	{
		Vec3 result = *this;
		result += InOther;
		return result;
	}

	Vec3 Vec3::operator-(const Vec3& InOther) const
	{
		Vec3 result = *this;
		result -= InOther;
		return result;
	}

	Vec3 Vec3::operator*(const Vec3& InOther) const
	{
		Vec3 result = *this;
		result *= InOther;
		return result;
	}

	Vec3 Vec3::operator/(const Vec3& InOther) const
	{
		Vec3 result = *this;
		result /= InOther;
		return result;
	}

}
