#pragma once

#include "AssetRegistry.hpp"

#include "Yuki/Core/Debug.hpp"

namespace Yuki {

	template<typename TAsset>
	class AssetImporter
	{
	public:
		AssetID Import(AssetRegistry& InRegistry, const std::filesystem::path& InFilePath);
		TAsset Load(AssetRegistry& InRegistry, AssetID InID);
	};

}
