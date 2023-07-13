#pragma once

#include "Yuki/Core/Debug.hpp"

#include "AssetRegistry.hpp"
#include "AssetDefinitions.hpp"

namespace Yuki {

	template<typename... TLambdas>
	struct ImageVisitor : TLambdas... { using TLambdas::operator()...; };
	template<typename... TLambdas>
	ImageVisitor(TLambdas...) -> ImageVisitor<TLambdas...>;

	template<typename TAsset>
	struct AssetImporter {};

	template<>
	struct AssetImporter<MeshAsset>
	{
		AssetID Import(AssetRegistry& InRegistry, const std::filesystem::path& InFilePath);
		bool Load(MeshAsset* InAsset, AssetRegistry& InRegistry, AssetID InID);
	};

	template<>
	struct AssetImporter<TextureAsset>
	{
		AssetID Import(AssetRegistry& InRegistry, const std::filesystem::path& InFilePath);
		AssetID Import(AssetRegistry& InRegistry, std::string_view InName, const std::byte* InData, size_t InDataSize);
		bool Load(TextureAsset* InAsset, AssetRegistry& InRegistry, AssetID InID);
	};


}
