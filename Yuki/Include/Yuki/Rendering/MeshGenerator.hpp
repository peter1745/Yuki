#pragma once

#include "Mesh.hpp"
#include "Yuki/Asset/AssetSystem.hpp"

namespace Yuki {

	class MeshGenerator
	{
	public:
		static AssetID GenerateCubeSphere(AssetSystem& InAssetSystem, float InRadius, uint32_t InSegments, float InUVMultiplier);
	};

}
