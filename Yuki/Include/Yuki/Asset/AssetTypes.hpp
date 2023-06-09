#pragma once

#include <string_view>

namespace Yuki {

	enum class AssetType
	{
		Unknown = -1,
		Mesh,
		Texture
	};

	namespace Utils {

		static constexpr std::string_view GetAssetTypeName(AssetType InType)
		{
			switch (InType)
			{
			case AssetType::Mesh: return "Mesh";
			default: return "Unknown";
			}
		}

		static constexpr AssetType GetAssetTypeFromName(std::string_view InType)
		{
			if (InType == "Mesh") return AssetType::Mesh;

			return AssetType::Unknown;
		}

	}

}
