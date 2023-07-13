#pragma once

#include "Yuki/Rendering/Mesh.hpp"

namespace Yuki {

	struct Asset {};

	struct MeshAsset : public Asset
	{
		MeshScene Scene;
	};

	struct TextureAsset : public Asset
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		std::byte* Data = nullptr;
	};

}
