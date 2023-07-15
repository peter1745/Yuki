#pragma once

#include "AssetImporter.hpp"

namespace Yuki {

	class AssetSystem
	{
	public:
		AssetSystem(AssetRegistry& InRegistry);

		template<std::derived_from<Asset> TAsset>
		TAsset* Request(AssetID InID)
		{
			if (m_Assets.contains(InID))
				return static_cast<TAsset*>(m_Assets[InID].get());

			if (Load<TAsset>(InID))
				return static_cast<TAsset*>(m_Assets.at(InID).get());

			return nullptr;
		}

		template<std::derived_from<Asset> TAsset>
		AssetID AddAsset(AssetType InType, TAsset* InAsset)
		{
			AssetID assetID(InType);
			m_Assets[assetID].reset(InAsset);
			return assetID;
		}

	private:
		bool LoadFromID(AssetID InID);

		template<std::derived_from<Asset> TAsset>
		bool Load(AssetID InID)
		{
			const auto& metadata = m_Registry[InID];
			for (const auto& dependency : metadata.Dependencies)
			{
				if (m_Assets.contains(dependency))
					continue;

				const auto& dependencyMetadata = m_Registry[dependency];
				if (!LoadFromID(dependency))
				{
					LogError("Failed to load asset {} due to dependency {} failing to load.", metadata.FilePath.string(), dependencyMetadata.FilePath.string());
					return false;
				}
			}

			LogInfo("Loading {}", metadata.FilePath.string());

			std::unique_ptr<TAsset> asset = std::make_unique<TAsset>();
			bool result = AssetImporter<TAsset>().Load(asset.get(), m_Registry, InID);
			if (result)
				m_Assets[InID] = std::move(asset);

			return result;
		}

	private:
		AssetRegistry& m_Registry;
		Map<AssetID, std::unique_ptr<Asset>, AssetIDHash> m_Assets;
	};

}
