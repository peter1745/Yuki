#pragma once

#include "Quat.hpp"

namespace Yuki::Math {
	
	struct Mat4
	{
		std::array<Vec4, 4> Columns;

		void SetIdentity();
		
		Mat4& operator*=(const Mat4& InOther);
		Mat4 operator*(const Mat4& InOther) const;

		Vec4& operator[](size_t InIndex) { return Columns[InIndex]; }
		const Vec4& operator[](size_t InIndex) const { return Columns[InIndex]; }

		static Mat4 Translation(const Vec3& InTranslation);
		static Mat4 Rotation(const Quat& InRotation);
		static Mat4 Scale(const Vec3& InScale);
		static Mat4 PerspectiveInfReversedZ(float InFovY, float InAspect, float InNearZ);

	};

}
