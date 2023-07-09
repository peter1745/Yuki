#pragma once

#include "AssetTypes.hpp"
#include "AssetID.hpp"

namespace Yuki {

	class AssetSystem
	{
	public:
		template<typename TAsset>
		requires std::derived_from<TAsset, Asset>
		const TAsset* Request(AssetID InID) const
		{
			if (!m_Assets.contains(InID))
				return nullptr;

			return m_Assets.at(InID).get();
		}

	private:
		Map<AssetID, std::unique_ptr<Asset>, AssetIDHash> m_Assets;
	};

}
