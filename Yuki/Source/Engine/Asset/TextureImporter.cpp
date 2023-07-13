#include "Asset/AssetImporter.hpp"

#include <stb_image/stb_image.h>

#include <fstream>

namespace Yuki {

	AssetID AssetImporter<TextureAsset>::Import(AssetRegistry& InRegistry, const std::filesystem::path& InFilePath)
	{
		int32_t width = 0;
		int32_t height = 0;
		auto* imageData = stbi_load(InFilePath.string().c_str(), &width, &height, nullptr, STBI_rgb_alpha);

		int32_t imageDataSize = width * height * 4;

		// TODO(Peter): Blit image based on import settings
		std::filesystem::path filepath = "Content" / InFilePath.stem();
		filepath.replace_extension(".asset");

		std::ofstream stream(filepath, std::ios::binary);
		stream.write(reinterpret_cast<const char*>(&width), sizeof(int32_t));
		stream.write(reinterpret_cast<const char*>(&height), sizeof(int32_t));
		stream.write(reinterpret_cast<const char*>(&imageDataSize), sizeof(int32_t));
		stream.write(reinterpret_cast<const char*>(imageData), imageDataSize);
		stream.close();

		return InRegistry.Register(AssetType::Texture, {
			.FilePath = filepath,
			.SourceFilePath = InFilePath
		});
	}

	AssetID AssetImporter<TextureAsset>::Import(AssetRegistry& InRegistry, std::string_view InName, const std::byte* InData, size_t InDataSize)
	{
		int32_t width = 0;
		int32_t height = 0;
		auto* imageData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(InData), int32_t(InDataSize), &width, &height, nullptr, STBI_rgb_alpha);

		int32_t imageDataSize = width * height * 4;

		// TODO(Peter): Blit image based on import settings
		std::filesystem::path filepath = "Content" / std::filesystem::path(InName);
		filepath.replace_extension(".asset");
		
		std::ofstream stream(filepath, std::ios::binary);
		stream.write(reinterpret_cast<const char*>(&width), sizeof(int32_t));
		stream.write(reinterpret_cast<const char*>(&height), sizeof(int32_t));
		stream.write(reinterpret_cast<const char*>(&imageDataSize), sizeof(int32_t));
		stream.write(reinterpret_cast<const char*>(imageData), imageDataSize);
		stream.close();

		return InRegistry.Register(AssetType::Texture, {
			.FilePath = filepath,
			.SourceFilePath = InName
		});
	}

	bool AssetImporter<TextureAsset>::Load(TextureAsset* InAsset, AssetRegistry& InRegistry, AssetID InID)
	{
		std::ifstream stream(InRegistry[InID].FilePath, std::ios::binary);

		if (!stream)
			return false;

		stream.read(reinterpret_cast<char*>(&InAsset->Width), sizeof(int32_t));
		stream.read(reinterpret_cast<char*>(&InAsset->Height), sizeof(int32_t));

		int32_t dataSize;
		stream.read(reinterpret_cast<char*>(&dataSize), sizeof(int32_t));

		InAsset->Data = new std::byte[dataSize];
		stream.read(reinterpret_cast<char*>(InAsset->Data), dataSize);

		return true;
	}

}
