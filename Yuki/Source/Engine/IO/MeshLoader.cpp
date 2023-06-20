#include "IO/MeshLoader.hpp"

#include <fastgltf/parser.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

namespace fastgltf {

	template<>
	struct ElementTraits<Yuki::Math::Vec3> : ElementTraitsBase<float, AccessorType::Vec3>
	{};

	template<>
	struct ElementTraits<Yuki::Math::Vec2> : ElementTraitsBase<float, AccessorType::Vec2>
	{};

}

namespace Yuki {

	MeshData MeshLoader::LoadGLTFMesh(const std::filesystem::path& InFilePath)
	{
		fastgltf::Parser parser;
		fastgltf::GltfDataBuffer dataBuffer;
		dataBuffer.loadFromFile(InFilePath);

		std::unique_ptr<fastgltf::glTF> gltfAsset;

		fastgltf::Options options = fastgltf::Options::DontRequireValidAssetMember |
			fastgltf::Options::LoadGLBBuffers |
			fastgltf::Options::LoadExternalBuffers |
			fastgltf::Options::DecomposeNodeMatrices |
			fastgltf::Options::LoadExternalImages;

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
		YUKI_VERIFY(gltfAsset->parse() == fastgltf::Error::None);

		auto asset = gltfAsset->getParsedAsset();

		MeshData result = {};

		YUKI_STOPWATCH_START();
		for (auto& mesh : asset->meshes)
		{
			for (auto& primitive : mesh.primitives)
			{
				if (primitive.attributes.find("POSITION") == primitive.attributes.end())
					continue;

				auto& positionAccessor = asset->accessors[primitive.attributes["POSITION"]];

				if (!primitive.indicesAccessor.has_value())
					break;

				size_t baseVertexOffset = result.Vertices.size();
				size_t vertexID = baseVertexOffset;
				result.Vertices.resize(baseVertexOffset + positionAccessor.count);

				{
					auto& indicesAccessor = asset->accessors[primitive.indicesAccessor.value()];
					fastgltf::iterateAccessor<uint32_t>(*asset, indicesAccessor, [&](uint32_t InIndex)
					{
						result.Indices.emplace_back(InIndex + uint32_t(baseVertexOffset));
					});
				}

				{
					fastgltf::iterateAccessor<Math::Vec3>(*asset, positionAccessor, [&](Math::Vec3 InPosition)
					{
						result.Vertices[vertexID++].Position = InPosition;
					});
					vertexID = baseVertexOffset;
				}

				if (primitive.attributes.contains("NORMAL"))
				{
					auto& normalsAccessor = asset->accessors[primitive.attributes["NORMAL"]];
					fastgltf::iterateAccessor<Math::Vec3>(*asset, normalsAccessor, [&](Math::Vec3 InNormal)
					{
						result.Vertices[vertexID++].Normal = InNormal;
					});
					vertexID = baseVertexOffset;
				}

				if (primitive.attributes.contains("TEXCOORD_0"))
				{
					auto& uvAccessor = asset->accessors[primitive.attributes["TEXCOORD_0"]];
					fastgltf::iterateAccessor<Math::Vec2>(*asset, uvAccessor, [&](Math::Vec2 InUV)
					{
						result.Vertices[vertexID++].UV = InUV;
					});
					vertexID = baseVertexOffset;
				}
			}
		}
		YUKI_STOPWATCH_STOP();

		return result;
	}

}
