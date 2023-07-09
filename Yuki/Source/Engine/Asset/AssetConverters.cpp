#include "Asset/AssetConverters.hpp"
#include "Asset/AssetTypes.hpp"
#include "Memory/Buffer.hpp"

#include <fastgltf/parser.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

#include <stb_image/stb_image.h>

namespace fastgltf {

	template<>
	struct ElementTraits<Yuki::Math::Vec3> : ElementTraitsBase<float, AccessorType::Vec3>
	{};

	template<>
	struct ElementTraits<Yuki::Math::Vec2> : ElementTraitsBase<float, AccessorType::Vec2>
	{};

}

namespace Yuki {

	void ProcessNodeHierarchy(fastgltf::Asset* InAsset, MeshScene& InScene, size_t InNodeIndex)
	{
		const auto& gltfNode = InAsset->nodes[InNodeIndex];
		auto TRS = std::get<fastgltf::Node::TRS>(gltfNode.transform);

		auto& node = InScene.Nodes.emplace_back();
		node.Translation = Math::Vec3(TRS.translation);
		node.Rotation = Math::Quat(TRS.rotation);
		node.Scale = Math::Vec3(TRS.scale);
		node.MeshIndex = gltfNode.meshIndex.has_value() ? static_cast<int32_t>(gltfNode.meshIndex.value()) : -1;

		for (auto childNodeIndex : gltfNode.children)
			ProcessNodeHierarchy(InAsset, InScene, childNodeIndex);
	}

	std::pair<std::filesystem::path, MeshScene> MeshConverter::Convert(const std::filesystem::path& InFilePath) const
	{
		fastgltf::Parser parser;
		fastgltf::GltfDataBuffer dataBuffer;
		dataBuffer.loadFromFile(InFilePath);

		std::unique_ptr<fastgltf::glTF> gltfAsset;

		fastgltf::Options options =
			fastgltf::Options::DontRequireValidAssetMember |
			fastgltf::Options::LoadGLBBuffers |
			fastgltf::Options::LoadExternalBuffers |
			fastgltf::Options::DecomposeNodeMatrices;

		switch (fastgltf::determineGltfFileType(&dataBuffer))
		{
		case fastgltf::GltfType::glTF:
		{
			gltfAsset = parser.loadGLTF(&dataBuffer, InFilePath.parent_path(), options);
			break;
		}
		case fastgltf::GltfType::GLB:
		{
			gltfAsset = parser.loadBinaryGLTF(&dataBuffer, InFilePath.parent_path(), options);
			break;
		}
		}

		YUKI_VERIFY(parser.getError() == fastgltf::Error::None);
		auto res = gltfAsset->parse();
		if (res != fastgltf::Error::None)
		{
			YUKI_VERIFY(false);
		}

		auto* parsedAsset = gltfAsset->getParsedAsset().release();
		gltfAsset.reset();

		fastgltf::Scene* scene = nullptr;
		if (parsedAsset->defaultScene.has_value())
			scene = &parsedAsset->scenes[parsedAsset->defaultScene.value()];
		else
			scene = &parsedAsset->scenes[0];

		YUKI_VERIFY(scene);

		MeshScene meshScene;
		meshScene.Materials.resize(parsedAsset->materials.size());
		meshScene.Meshes.resize(parsedAsset->meshes.size());
		meshScene.Nodes.resize(parsedAsset->nodes.size());

		for (size_t i = 0; i < parsedAsset->materials.size(); i++)
		{
			auto& material = meshScene.Materials[i];
			const auto& gltfMaterial = parsedAsset->materials[i];

			if (!gltfMaterial.pbrData.has_value())
				continue;

			const auto& pbrData = gltfMaterial.pbrData.value();

			uint32_t r = uint32_t(pbrData.baseColorFactor[0]) * 255;
			uint32_t g = uint32_t(pbrData.baseColorFactor[1]) * 255;
			uint32_t b = uint32_t(pbrData.baseColorFactor[2]) * 255;
			uint32_t a = uint32_t(pbrData.baseColorFactor[3]) * 255;
			material.AlbedoColor = (r << 24) | (g << 16) | (b << 8) | a;

			if (pbrData.baseColorTexture.has_value())
			{
				material.AlbedoTextureIndex = static_cast<int32_t>(pbrData.baseColorTexture.value().textureIndex);
			}
		}

		for (size_t i = 0; i < parsedAsset->meshes.size(); i++)
		{
			auto& gltfMesh = parsedAsset->meshes[i];
			auto& sourceData = meshScene.Meshes[i];

			for (auto& primitive : gltfMesh.primitives)
			{
				if (primitive.attributes.find("POSITION") == primitive.attributes.end())
					continue;

				auto& positionAccessor = parsedAsset->accessors[primitive.attributes["POSITION"]];

				if (!primitive.indicesAccessor.has_value())
					break;

				size_t baseVertexOffset = sourceData.Vertices.size();
				size_t vertexID = baseVertexOffset;
				sourceData.Vertices.resize(baseVertexOffset + positionAccessor.count);

				auto& indicesAccessor = parsedAsset->accessors[primitive.indicesAccessor.value()];
				fastgltf::iterateAccessor<uint32_t>(*parsedAsset, indicesAccessor, [&](uint32_t InIndex)
				{
					sourceData.Indices.emplace_back(InIndex + uint32_t(baseVertexOffset));
				});

				fastgltf::iterateAccessor<Math::Vec3>(*parsedAsset, positionAccessor, [&](Math::Vec3 InPosition)
				{
					sourceData.Vertices[vertexID].Position = InPosition;
					sourceData.Vertices[vertexID].MaterialIndex = uint32_t(primitive.materialIndex.value_or(0));
					vertexID++;
				});
				vertexID = baseVertexOffset;

				if (primitive.attributes.contains("NORMAL"))
				{
					auto& normalsAccessor = parsedAsset->accessors[primitive.attributes["NORMAL"]];
					fastgltf::iterateAccessor<Math::Vec3>(*parsedAsset, normalsAccessor, [&](Math::Vec3 InNormal)
					{
						sourceData.Vertices[vertexID++].Normal = InNormal;
					});
					vertexID = baseVertexOffset;
				}

				if (primitive.attributes.contains("TEXCOORD_0"))
				{
					auto& uvAccessor = parsedAsset->accessors[primitive.attributes["TEXCOORD_0"]];
					fastgltf::iterateAccessor<Math::Vec2>(*parsedAsset, uvAccessor, [&](Math::Vec2 InUV)
					{
						sourceData.Vertices[vertexID++].UV = InUV;
					});
					vertexID = baseVertexOffset;
				}
			}
		}

		for (auto nodeIndex : scene->nodeIndices)
			ProcessNodeHierarchy(parsedAsset, meshScene, nodeIndex);

		auto meshTypeName = Utils::GetAssetTypeName(AssetType::Mesh);

		size_t dataSize = 4 * sizeof(size_t);
		dataSize += meshTypeName.size();
		dataSize += meshScene.Materials.size() * sizeof(MaterialData);
		dataSize += meshScene.Nodes.size() * sizeof(MeshNode);

		for (const auto& meshSource : meshScene.Meshes)
		{
			dataSize += meshSource.Vertices.size() * sizeof(Vertex);
			dataSize += meshSource.Indices.size() * sizeof(uint32_t);
		}

		Buffer buffer(dataSize);
		buffer.Write(meshTypeName.length());
		buffer.Write(meshTypeName);

		buffer.Write(meshScene.Materials.size());
		buffer.Write(meshScene.Nodes.size());
		buffer.Write(meshScene.Meshes.size());

		buffer.WriteArray(meshScene.Materials);
		buffer.WriteArray(meshScene.Nodes);

		for (const auto& meshSource : meshScene.Meshes)
		{
			buffer.WriteArray(meshSource.Vertices);
			buffer.WriteArray(meshSource.Indices);
		}

		std::filesystem::path filepath = "Content" / InFilePath.filename();
		filepath.replace_extension(".asset");

		std::ofstream stream(filepath, std::ios::binary);
		stream.write(reinterpret_cast<const char*>(buffer.Data()), dataSize);
		stream.close();

		delete parsedAsset;

		return { filepath, meshScene };
	}

}
