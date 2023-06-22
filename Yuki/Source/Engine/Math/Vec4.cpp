#include "Math/Vec4.hpp"

namespace Yuki::Math {

	Vec4& Vec4::operator+=(const Vec4& InOther)
	{
		for (size_t i = 0; i < 4; i++)
			Values[i] += InOther[i];
		return *this;
	}

	Vec4& Vec4::operator-=(const Vec4& InOther)
	{
		for (size_t i = 0; i < 4; i++)
			Values[i] -= InOther[i];
		return *this;
	}

	Vec4& Vec4::operator*=(const Vec4& InOther)
	{
		for (size_t i = 0; i < 4; i++)
			Values[i] *= InOther[i];
		return *this;
	}

	Vec4& Vec4::operator/=(const Vec4& InOther)
	{
		for (size_t i = 0; i < 4; i++)
			Values[i] /= InOther[i];
		return *this;
	}

	Vec4& Vec4::operator+=(const float InOther)
	{
		for (size_t i = 0; i < 4; i++)
			Values[i] += InOther;
		return *this;
	}

	Vec4& Vec4::operator-=(const float InOther)
	{
		for (size_t i = 0; i < 4; i++)
			Values[i] -= InOther;
		return *this;
	}

	Vec4& Vec4::operator*=(const float InOther)
	{
		for (size_t i = 0; i < 4; i++)
			Values[i] *= InOther;
		return *this;
	}

	Vec4& Vec4::operator/=(const float InOther)
	{
		for (size_t i = 0; i < 4; i++)
			Values[i] /= InOther;
		return *this;
	}

	Vec4 Vec4::operator+(const Vec4& InOther) const
	{
		Vec4 result = *this;
		result += InOther;
		return result;
	}

	Vec4 Vec4::operator-(const Vec4& InOther) const
	{
		Vec4 result = *this;
		result -= InOther;
		return result;
	}

	Vec4 Vec4::operator*(const Vec4& InOther) const
	{
		Vec4 result = *this;
		result *= InOther;
		return result;
	}

	Vec4 Vec4::operator/(const Vec4& InOther) const
	{
		Vec4 result = *this;
		result /= InOther;
		return result;
	}

	Vec4 Vec4::operator+(const float InOther) const
	{
		Vec4 result = *this;
		result += InOther;
		return result;
	}

	Vec4 Vec4::operator-(const float InOther) const
	{
		Vec4 result = *this;
		result -= InOther;
		return result;
	}

	Vec4 Vec4::operator*(const float InOther) const
	{
		Vec4 result = *this;
		result *= InOther;
		return result;
	}

	Vec4 Vec4::operator/(const float InOther) const
	{
		Vec4 result = *this;
		result /= InOther;
		return result;
	}

}
