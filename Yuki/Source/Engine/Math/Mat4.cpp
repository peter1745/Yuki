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

	Mat4& Mat4::operator*=(const float InScalar)
	{
		for (size_t i = 0; i < 4; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				Columns[i][j] *= InScalar;
			}
		}

		return *this;
	}

	Mat4 Mat4::operator*(const Mat4& InOther) const
	{
		Mat4 result = *this;
		result *= InOther;
		return result;
	}

	Mat4 Mat4::operator*(const float InScalar) const
	{
		Mat4 result = *this;
		result *= InScalar;
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
		result[1][1] = -f;
		result[3][2] = InNearZ;
		result[2][3] = -1.0f;

		return result;
	}

	Mat4 Mat4::InvertAffine(const Mat4& InMatrix)
	{
		float Coef00 = InMatrix[2][2] * InMatrix[3][3] - InMatrix[3][2] * InMatrix[2][3];
		float Coef02 = InMatrix[1][2] * InMatrix[3][3] - InMatrix[3][2] * InMatrix[1][3];
		float Coef03 = InMatrix[1][2] * InMatrix[2][3] - InMatrix[2][2] * InMatrix[1][3];

		float Coef04 = InMatrix[2][1] * InMatrix[3][3] - InMatrix[3][1] * InMatrix[2][3];
		float Coef06 = InMatrix[1][1] * InMatrix[3][3] - InMatrix[3][1] * InMatrix[1][3];
		float Coef07 = InMatrix[1][1] * InMatrix[2][3] - InMatrix[2][1] * InMatrix[1][3];

		float Coef08 = InMatrix[2][1] * InMatrix[3][2] - InMatrix[3][1] * InMatrix[2][2];
		float Coef10 = InMatrix[1][1] * InMatrix[3][2] - InMatrix[3][1] * InMatrix[1][2];
		float Coef11 = InMatrix[1][1] * InMatrix[2][2] - InMatrix[2][1] * InMatrix[1][2];

		float Coef12 = InMatrix[2][0] * InMatrix[3][3] - InMatrix[3][0] * InMatrix[2][3];
		float Coef14 = InMatrix[1][0] * InMatrix[3][3] - InMatrix[3][0] * InMatrix[1][3];
		float Coef15 = InMatrix[1][0] * InMatrix[2][3] - InMatrix[2][0] * InMatrix[1][3];

		float Coef16 = InMatrix[2][0] * InMatrix[3][2] - InMatrix[3][0] * InMatrix[2][2];
		float Coef18 = InMatrix[1][0] * InMatrix[3][2] - InMatrix[3][0] * InMatrix[1][2];
		float Coef19 = InMatrix[1][0] * InMatrix[2][2] - InMatrix[2][0] * InMatrix[1][2];

		float Coef20 = InMatrix[2][0] * InMatrix[3][1] - InMatrix[3][0] * InMatrix[2][1];
		float Coef22 = InMatrix[1][0] * InMatrix[3][1] - InMatrix[3][0] * InMatrix[1][1];
		float Coef23 = InMatrix[1][0] * InMatrix[2][1] - InMatrix[2][0] * InMatrix[1][1];

		Vec4 Fac0(Coef00, Coef00, Coef02, Coef03);
		Vec4 Fac1(Coef04, Coef04, Coef06, Coef07);
		Vec4 Fac2(Coef08, Coef08, Coef10, Coef11);
		Vec4 Fac3(Coef12, Coef12, Coef14, Coef15);
		Vec4 Fac4(Coef16, Coef16, Coef18, Coef19);
		Vec4 Fac5(Coef20, Coef20, Coef22, Coef23);

		Vec4 Vec0(InMatrix[1][0], InMatrix[0][0], InMatrix[0][0], InMatrix[0][0]);
		Vec4 Vec1(InMatrix[1][1], InMatrix[0][1], InMatrix[0][1], InMatrix[0][1]);
		Vec4 Vec2(InMatrix[1][2], InMatrix[0][2], InMatrix[0][2], InMatrix[0][2]);
		Vec4 Vec3(InMatrix[1][3], InMatrix[0][3], InMatrix[0][3], InMatrix[0][3]);

		Vec4 Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
		Vec4 Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
		Vec4 Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
		Vec4 Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

		Vec4 SignA(+1, -1, +1, -1);
		Vec4 SignB(-1, +1, -1, +1);
		Mat4 Inverse{ Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB };

		Vec4 Row0(Inverse[0][0], Inverse[1][0], Inverse[2][0], Inverse[3][0]);

		Vec4 Dot0(InMatrix[0] * Row0);
		float Dot1 = (Dot0.X + Dot0.Y) + (Dot0.Z + Dot0.W);

		float OneOverDeterminant = static_cast<float>(1) / Dot1;

		return Inverse * OneOverDeterminant;
	}

}
