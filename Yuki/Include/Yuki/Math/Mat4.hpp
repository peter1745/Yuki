#pragma once

#include "Quat.hpp"

namespace Yuki::Math {
	
	struct Mat4
	{
		std::array<Vec4, 4> Columns;

		void SetIdentity();

		static Mat4 InvertAffine(const Mat4& InMatrix);
		
		Mat4& operator*=(const Mat4& InOther);
		Mat4& operator*=(const float InScalar);
		Mat4 operator*(const Mat4& InOther) const;
		Mat4 operator*(const float InScalar) const;

		Vec4& operator[](size_t InIndex) { return Columns[InIndex]; }
		const Vec4& operator[](size_t InIndex) const { return Columns[InIndex]; }

		static Mat4 Translation(const Vec3& InTranslation);
		static Mat4 Rotation(const Quat& InRotation);
		static Mat4 Scale(const Vec3& InScale);
		static Mat4 PerspectiveInfReversedZ(float InFovY, float InAspect, float InNearZ);

		static Mat4 LookAt(const Vec3& InEye, const Vec3& InCenter, const Vec3& InUp);

	};

}
