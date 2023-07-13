#pragma once

#include "AssetImporter.hpp"

namespace Yuki {

	class AssetSystem
	{
	public:
		AssetSystem(AssetRegistry& InRegistry);

		template<std::derived_from<Asset> TAsset>
		void Request(AssetID InID, std::function<void(const TAsset&)> InCallback)
		{
			if (m_Assets.contains(InID))
			{
				InCallback(*static_cast<const TAsset*>(m_Assets[InID].get()));
				return;
			}

			std::unique_ptr<TAsset> asset = std::make_unique<TAsset>();
			if (AssetImporter<TAsset>().Load(asset.get(), m_Registry, InID))
			{
				m_Assets[InID] = std::move(asset);
				InCallback(*static_cast<const TAsset*>(m_Assets.at(InID).get()));
			}
		}

	private:
		AssetRegistry& m_Registry;
		Map<AssetID, std::unique_ptr<Asset>, AssetIDHash> m_Assets;
	};

}
