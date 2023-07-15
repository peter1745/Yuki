#include "Math/Vec3.hpp"
#include "Math/Math.hpp"

namespace Yuki::Math {

	float Vec3::Dot(const Vec3& InOther) const
	{
		return X * InOther.X + Y * InOther.Y + Z * InOther.Z;
	}

	float Vec3::Length() const
	{
		return Math::Sqrt(Dot(*this));
	}

	void Vec3::Normalize()
	{
		float length = Length();
		float invLength = 1.0f / length;
		X *= invLength;
		Y *= invLength;
		Z *= invLength;
	}

    Vec3 Vec3::Normalized() const
    {
		Vec3 result = *this;
		result.Normalize();
        return result;
    }

    Vec3 Vec3::Cross(const Vec3& InOther) const
    {
        Vec3 result;
		result.X = Y * InOther.Z - Z * InOther.Y;
		result.Y = Z * InOther.X - X * InOther.Z;
		result.Z = X * InOther.Y - Y * InOther.X;
		return result;
    }

	Vec3 Vec3::Slerp(const Vec3& InStart, const Vec3& InEnd, float InPercent)
	{
		float dot = Math::Clamp(InStart.Dot(InEnd), -1.0f, 1.0f);
		float theta = Math::Acos(dot) * InPercent;
		auto relative = InEnd - InStart * dot;

		relative.Normalize();

		return (InStart * Math::Cos(theta)) + (relative * Math::Sin(theta));
	}

    Vec3 &Vec3::operator+=(const Vec3 &InOther)
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

	 Vec3 &Vec3::operator+=(const float InScalar)
    {
		for (size_t i = 0; i < 3; i++)
			Values[i] += InScalar;
		return *this;
	}

	Vec3& Vec3::operator-=(const float InScalar)
	{
		for (size_t i = 0; i < 3; i++)
			Values[i] -= InScalar;
		return *this;
	}

	Vec3& Vec3::operator*=(const float InScalar)
	{
		for (size_t i = 0; i < 3; i++)
			Values[i] *= InScalar;
		return *this;
	}

	Vec3& Vec3::operator/=(const float InScalar)
	{
		for (size_t i = 0; i < 3; i++)
			Values[i] /= InScalar;
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

	Vec3 Vec3::operator+(const float InScalar) const
	{
		Vec3 result = *this;
		result += InScalar;
		return result;
	}

	Vec3 Vec3::operator-(const float InScalar) const
	{
		Vec3 result = *this;
		result -= InScalar;
		return result;
	}

	Vec3 Vec3::operator*(const float InScalar) const
	{
		Vec3 result = *this;
		result *= InScalar;
		return result;
	}

	Vec3 Vec3::operator/(const float InScalar) const
	{
		Vec3 result = *this;
		result /= InScalar;
		return result;
	}

}
