#include "IO/MeshLoader.hpp"

#include "Rendering/RHI/RenderContext.hpp"

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

	static Buffer* s_StagingBuffer = nullptr;

	void ProcessNodeHierarchy(fastgltf::Asset* InAsset, LoadedMesh& InMeshStorage, fastgltf::Node& InNode)
	{
		if (InNode.meshIndex.has_value())
		{
			auto& meshInstance = InMeshStorage.Instances.emplace_back();
			meshInstance.SourceMesh = &InMeshStorage.Meshes[InNode.meshIndex.value()];
		}

		for (auto childNodeIndex : InNode.children)
			ProcessNodeHierarchy(InAsset, InMeshStorage, InAsset->nodes[childNodeIndex]);
	}

	LoadedMesh MeshLoader::LoadGLTFMesh(RenderContext* InContext, const std::filesystem::path& InFilePath)
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

		LoadedMesh result = {};
		result.Meshes.reserve(asset->meshes.size());

		YUKI_STOPWATCH_START();
		for (auto& mesh : asset->meshes)
		{
			Mesh& meshData = result.Meshes.emplace_back();

			for (auto& primitive : mesh.primitives)
			{
				if (primitive.attributes.find("POSITION") == primitive.attributes.end())
					continue;

				auto& positionAccessor = asset->accessors[primitive.attributes["POSITION"]];

				if (!primitive.indicesAccessor.has_value())
					break;

				size_t baseVertexOffset = meshData.Vertices.size();
				size_t vertexID = baseVertexOffset;
				meshData.Vertices.resize(baseVertexOffset + positionAccessor.count);

				{
					auto& indicesAccessor = asset->accessors[primitive.indicesAccessor.value()];
					fastgltf::iterateAccessor<uint32_t>(*asset, indicesAccessor, [&](uint32_t InIndex)
					{
						meshData.Indices.emplace_back(InIndex + uint32_t(baseVertexOffset));
					});
				}

				{
					fastgltf::iterateAccessor<Math::Vec3>(*asset, positionAccessor, [&](Math::Vec3 InPosition)
					{
						meshData.Vertices[vertexID++].Position = InPosition;
					});
					vertexID = baseVertexOffset;
				}

				if (primitive.attributes.contains("NORMAL"))
				{
					auto& normalsAccessor = asset->accessors[primitive.attributes["NORMAL"]];
					fastgltf::iterateAccessor<Math::Vec3>(*asset, normalsAccessor, [&](Math::Vec3 InNormal)
					{
						meshData.Vertices[vertexID++].Normal = InNormal;
					});
					vertexID = baseVertexOffset;
				}

				if (primitive.attributes.contains("TEXCOORD_0"))
				{
					auto& uvAccessor = asset->accessors[primitive.attributes["TEXCOORD_0"]];
					fastgltf::iterateAccessor<Math::Vec2>(*asset, uvAccessor, [&](Math::Vec2 InUV)
					{
						meshData.Vertices[vertexID++].UV = InUV;
					});
					vertexID = baseVertexOffset;
				}
			}

			if (s_StagingBuffer == nullptr)
			{
				Yuki::BufferInfo bufferInfo =
				{
					.Type = BufferType::StagingBuffer,
					.Size = 100 * 1024 * 1024,
					.PersistentlyMapped = true
				};
				s_StagingBuffer = InContext->CreateBuffer(bufferInfo);
			}

			{
				if (result.Meshes.size() > 114)
				{
					for (size_t i = 0; i < 6; i++)
						LogInfo("V{}: {}", i, meshData.Vertices[i].Position);
				}
				uint32_t vertexDataSize = sizeof(Vertex) * uint32_t(meshData.Vertices.size());

				s_StagingBuffer->SetData(meshData.Vertices.data(), vertexDataSize);

				Yuki::BufferInfo bufferInfo =
				{
					.Type = BufferType::VertexBuffer,
					.Size = vertexDataSize
				};
				meshData.VertexBuffer = InContext->CreateBuffer(bufferInfo);
				YUKI_VERIFY(bufferInfo.Size < 100 * 1024 * 1024);
				meshData.VertexBuffer->UploadData(s_StagingBuffer);
			}

			{
				uint32_t indexDataSize = sizeof(uint32_t) * uint32_t(meshData.Indices.size());

				s_StagingBuffer->SetData(meshData.Indices.data(), indexDataSize);

				Yuki::BufferInfo bufferInfo =
				{
					.Type = BufferType::IndexBuffer,
					.Size = indexDataSize
				};
				meshData.IndexBuffer = InContext->CreateBuffer(bufferInfo);
				YUKI_VERIFY(bufferInfo.Size < 100 * 1024 * 1024);
				meshData.IndexBuffer->UploadData(s_StagingBuffer);
			}
		}
		YUKI_STOPWATCH_STOP();

		fastgltf::Scene* scene = nullptr;
		if (asset->defaultScene.has_value())
			scene = &asset->scenes[asset->defaultScene.value()];
		else
			scene = &asset->scenes[0];
		YUKI_VERIFY(scene);

		result.Instances.reserve(scene->nodeIndices.size());

		for (auto nodeIndex : scene->nodeIndices)
		{
			auto& rootNode = asset->nodes[nodeIndex];
			ProcessNodeHierarchy(asset.get(), result, rootNode);
		}

		return result;
	}

}
