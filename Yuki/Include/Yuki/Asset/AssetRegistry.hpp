#pragma once

#include "AssetSystem.hpp"

#include <filesystem>

namespace Yuki {

	struct AssetMetadata
	{
		std::filesystem::path FilePath = "";
		std::filesystem::path SourceFilePath = "";
	};

	class AssetRegistry
	{
	public:
		AssetRegistry(const std::filesystem::path& InFilePath);

		AssetID Register(AssetType InType, const AssetMetadata& InMetadata);
		void Serialize();

		const AssetMetadata& operator[](AssetID InID) const { return m_Metadata.at(InID); }

		template<typename TFunction>
		void ForEach(TFunction&& InFunction)
		{
			for (const auto&[handle, metadata] : m_Metadata)
				InFunction(handle, metadata);
		}

	private:
		Map<AssetID, AssetMetadata, AssetIDHash> m_Metadata;
	};

}
