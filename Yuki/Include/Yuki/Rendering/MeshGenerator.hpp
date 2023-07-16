#pragma once

#include "Mesh.hpp"
#include "Yuki/Asset/AssetSystem.hpp"

namespace Yuki {

	class MeshGenerator
	{
	public:
		static AssetID GenerateCubeSphere(AssetSystem& InAssetSystem, float InRadius, uint32_t InSegments, float InUVMultiplier);

		// Icosphere implementation inspired by Sebastian Lague:
		//		YouTube Video: https://www.youtube.com/watch?v=lctXaT9pxA0
		//		GitHub: https://github.com/SebLague/Solar-System/blob/Development/Assets/Scripts/Celestial/SphereMesh.cs
		static AssetID GenerateIcosphere(AssetSystem& InAssetSystem, uint32_t InSubdivisions, float InUVMultiplier);
	};

}
