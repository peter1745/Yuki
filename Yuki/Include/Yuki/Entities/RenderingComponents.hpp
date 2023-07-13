#pragma once

#include "Yuki/Asset/AssetID.hpp"
#include "Yuki/Math/Mat4.hpp"

namespace Yuki::Entities {

	struct CameraComponent
	{
		Math::Mat4 ProjectionMatrix;
		Math::Mat4 ViewMatrix;
	};

	struct MeshComponent
	{
		AssetID MeshID;
		size_t MeshIndex;
	};

}
