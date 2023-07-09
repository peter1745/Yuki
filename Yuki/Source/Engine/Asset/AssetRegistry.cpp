#include "Asset/AssetRegistry.hpp"

#include <nlohmann/json.hpp>
#include <simdjson.h>

namespace Yuki {

	AssetRegistry::AssetRegistry(const std::filesystem::path& InFilePath)
	{
		if (std::filesystem::exists(InFilePath))
		{
			simdjson::ondemand::parser parser;
			simdjson::padded_string json = simdjson::padded_string::load(InFilePath.string());
			simdjson::ondemand::document doc = parser.iterate(json);
			for (auto metadataJson : doc.get_array())
			{
				AssetID handle(metadataJson["Handle"].get_uint64().value());
				
				AssetMetadata metadata =
				{
					.FilePath = metadataJson["FilePath"].get_string().value(),
					.SourceFilePath = metadataJson["SourceFilePath"].get_string().value(),
				};

				m_Metadata[handle] = std::move(metadata);
			}
		}
	}

	AssetID AssetRegistry::Register(AssetType InType, const AssetMetadata& InMetadata)
	{
		AssetID id(InType);
		m_Metadata[id] = InMetadata;
		return id;
	}

	void AssetRegistry::Serialize()
	{
		nlohmann::json json;

		for (const auto&[handle, metadata] : m_Metadata)
		{
			auto metadataObject = nlohmann::json::object({});
			metadataObject["Handle"] = static_cast<uint64_t>(handle);
			metadataObject["FilePath"] = metadata.FilePath;
			metadataObject["SourceFilePath"] = metadata.SourceFilePath;
			json.push_back(metadataObject);
		}

		std::ofstream stream("Content/AssetRegistry.json");
		stream << std::setw(4) << json;
		stream.close();
	}

}
