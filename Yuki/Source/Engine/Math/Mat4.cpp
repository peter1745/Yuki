#include "Math/Mat4.hpp"

namespace Yuki::Math {

	void Mat4::SetIdentity()
	{
		Columns = {
			Vec4{ 1.0f, 0.0f, 0.0f, 0.0f },
			Vec4{ 0.0f, 1.0f, 0.0f, 0.0f },
			Vec4{ 0.0f, 0.0f, 1.0f, 0.0f },
			Vec4{ 0.0f, 0.0f, 0.0f, 1.0f },
		};
	}

	Mat4& Mat4::operator*=(const Mat4& InOther)
	{
		const Vec4 c0 = Columns[0];
		const Vec4 c1 = Columns[1];
		const Vec4 c2 = Columns[2];
		const Vec4 c3 = Columns[3];

		Columns[0] = c0 * InOther[0][0] + c1 * InOther[0][1] + c2 * InOther[0][2] + c3 * InOther[0][3];
		Columns[1] = c0 * InOther[1][0] + c1 * InOther[1][1] + c2 * InOther[1][2] + c3 * InOther[1][3];
		Columns[2] = c0 * InOther[2][0] + c1 * InOther[2][1] + c2 * InOther[2][2] + c3 * InOther[2][3];
		Columns[3] = c0 * InOther[3][0] + c1 * InOther[3][1] + c2 * InOther[3][2] + c3 * InOther[3][3];

		return *this;
	}

	Mat4 Mat4::operator*(const Mat4& InOther) const
	{
		Mat4 result = *this;
		result *= InOther;
		return result;
	}

	Mat4 Mat4::Translation(const Vec3& InTranslation)
	{
		Mat4 result;
		result.SetIdentity();
		result[3] = Vec4(InTranslation, 1.0f);
		return result;
	}

	Mat4 Mat4::Rotation(const Quat& InRotation)
	{
		Mat4 result;
		result.SetIdentity();

		float x = InRotation.Value[0];
		float y = InRotation.Value[1];
		float z = InRotation.Value[2];
		float w = InRotation.Value[3];

		float xx = x * x;
		float xy = x * y;
		float xz = x * z;

		float wx = w * x;
		float wy = w * y;
		float wz = w * z;

		float yy = y * y;
		float yz = y * z;

		float zz = z * z;

		result[0][0] = 1.0f - 2.0f * (yy + zz);
		result[0][1] = 2.0f * (xy + wz);
		result[0][2] = 2.0f * (xz - wy);

		result[1][0] = 2.0f * (xy - wz);
		result[1][1] = 1.0f - 2.0f * (xx + zz);
		result[1][2] = 2.0f * (yz + wx);

		result[2][0] = 2.0f * (xz + wy);
		result[2][1] = 2.0f * (yz - wx);
		result[2][2] = 1.0f - 2.0f * (xx + yy);

		return result;
	}

	Mat4 Mat4::Scale(const Vec3& InScale)
	{
		Mat4 result;
		result.SetIdentity();
		result[0][0] = InScale[0];
		result[1][1] = InScale[1];
		result[2][2] = InScale[2];
		return result;
	}

	Mat4 Mat4::PerspectiveInfReversedZ(float InFovY, float InAspect, float InNearZ)
	{
		float f = 1.0f / std::tanf(InFovY / 2.0f);

		Mat4 result = {};
		result[0][0] = f / InAspect;
		result[1][1] = f;
		result[3][2] = InNearZ;
		result[2][3] = -1.0f;

		return result;
	}

}
