#pragma once

#include "Yuki/Math/Mat4.hpp"
//#include "Yuki/Rendering/MeshData.hpp"

namespace Yuki::Entities {

	struct CameraComponent
	{
		Math::Mat4 ProjectionMatrix;
		Math::Mat4 ViewMatrix;
	};

	struct MeshComponent
	{
		//MeshHandle Value{};
	};

}
