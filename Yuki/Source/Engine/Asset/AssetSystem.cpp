#include "Asset/AssetSystem.hpp"

namespace Yuki {

	AssetSystem::AssetSystem(AssetRegistry& InRegistry)
		: m_Registry(InRegistry)
	{
	}

	bool AssetSystem::LoadFromID(AssetID InID)
	{
		switch (InID.GetType())
		{
		case AssetType::Unknown: return false;
		case AssetType::Mesh: return Load<MeshAsset>(InID);
		case AssetType::Texture: return Load<TextureAsset>(InID);
		default:
			break;
		}

		return false;
	}

}
