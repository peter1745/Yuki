#pragma once

#include "Yuki/Asset/AssetID.hpp"
#include "Yuki/Math/Mat4.hpp"

namespace Yuki::Components {

	struct Camera
	{
		Math::Mat4 ProjectionMatrix;
		Math::Mat4 ViewMatrix;
	};

	struct Mesh
	{
		AssetID MeshID;
		size_t MeshIndex;
	};

}
