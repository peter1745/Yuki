#pragma once

#include "Yuki/Asset/AssetID.hpp"

namespace Yuki::Components {

	struct StarGenerator
	{
		uint32_t MeshSubdivisions = 16;
		float UVMultiplier = 1.0f;

		uint32_t NoiseSize = 512;
		float NoiseFrequency = 0.01f;
		bool RandomSeed = true;
		int32_t Seed = 0;

		bool Regenerate = false;

		// Runtime Data
		AssetID MeshID{ 0 };
		AssetID NoiseTextureID{ 0 };
	};
}
