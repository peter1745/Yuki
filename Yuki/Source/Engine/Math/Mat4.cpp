#include "Mat4.hpp"

namespace Yuki::Math {

	Mat4::Mat4()
	{
		Columns[0] = { 1.0f, 0.0f, 0.0f, 0.0f };
		Columns[1] = { 0.0f, 1.0f, 0.0f, 0.0f };
		Columns[2] = { 0.0f, 0.0f, 1.0f, 0.0f };
		Columns[3] = { 0.0f, 0.0f, 0.0f, 1.0f };
	}

	Mat4 Mat4::PerspectiveInfReversedZ(FPType InFovY, FPType InAspectRatio, FPType InNearZ)
	{
		Mat4 Result;

		FPType F = FPType(1.0f / std::tanf(InFovY * 0.5f));
		Result.Columns[0].X = F / InAspectRatio;
		Result.Columns[1].Y = -F;
		Result.Columns[2].Z = InNearZ;
		Result.Columns[3].W = -1.0f;

		return Result;
	}

	Mat4 Mat4::Translation(const Vec3& InTranslation)
	{
		Mat4 Result;
		Result.Columns[3] = { InTranslation.X, InTranslation.Y, InTranslation.Z, 1.0f };
		return Result;
	}

}
