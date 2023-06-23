#include "IO/MeshLoader.hpp"

//#include "Rendering/RHI/RenderContext.hpp"

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

	void ProcessNodeHierarchy(fastgltf::Asset* InAsset, LoadedMesh& InMeshStorage, fastgltf::Node& InNode, const Math::Mat4& InParentTransform)
	{
		auto& TRS = std::get<fastgltf::Node::TRS>(InNode.transform);
		Math::Mat4 modelTransform = Math::Mat4::Translation(Math::Vec3{TRS.translation}) * Math::Mat4::Rotation(Math::Quat{ TRS.rotation }) * Math::Mat4::Scale(Math::Vec3{TRS.scale});
		Math::Mat4 transform = InParentTransform * modelTransform;

		if (InNode.meshIndex.has_value())
		{
			auto& meshInstance = InMeshStorage.Instances.emplace_back();
			meshInstance.SourceMesh = &InMeshStorage.Meshes[InNode.meshIndex.value()];
			meshInstance.Transform = transform;
		}

		for (auto childNodeIndex : InNode.children)
			ProcessNodeHierarchy(InAsset, InMeshStorage, InAsset->nodes[childNodeIndex], transform);
	}

	void ProcessMaterials(RenderContext* InContext, fastgltf::Asset* InAsset, const std::filesystem::path& InBasePath, LoadedMesh& InMeshData)
	{
		InMeshData.LoadedImages.resize(InAsset->textures.size());

#pragma omp parallel for
		for (size_t i = 0; i < InAsset->textures.size(); i++)
		{
			YUKI_STOPWATCH_START_N("Load Texture (CPU)");
			auto& textureInfo = InAsset->textures[i];

			YUKI_VERIFY(textureInfo.imageIndex.has_value());
			auto& imageInfo = InAsset->images[textureInfo.imageIndex.value()];

			int width, height;
			stbi_uc* imageData = nullptr;
			std::visit(ImageVisitor
			{
				[&](fastgltf::sources::URI& InURI)
				{
					imageData = stbi_load(fmt::format("{}/{}", InBasePath.string(), InURI.uri.path()).c_str(), &width, &height, nullptr, STBI_rgb_alpha);
				},
				[&](fastgltf::sources::Vector& InVector)
				{
					imageData = stbi_load_from_memory(InVector.bytes.data(), uint32_t(InVector.bytes.size()), &width, &height, nullptr, STBI_rgb_alpha);
				},
				[&](fastgltf::sources::ByteView& InByteView)
				{
					imageData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(InByteView.bytes.data()), uint32_t(InByteView.bytes.size()), &width, &height, nullptr, STBI_rgb_alpha);
				},
				[&](fastgltf::sources::BufferView& InBufferView)
				{
					auto& view = InAsset->bufferViews[InBufferView.bufferViewIndex];
					auto& buffer = InAsset->buffers[view.bufferIndex];
					auto* bytes = fastgltf::DefaultBufferDataAdapter{}(buffer)+view.byteOffset;
					imageData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(bytes), uint32_t(view.byteLength), &width, &height, nullptr, STBI_rgb_alpha);
				},
				[&](auto&) {}
			}, imageInfo.data);

			YUKI_VERIFY(imageData);

			auto& loadedImage = InMeshData.LoadedImages[i];
			loadedImage.Width = uint32_t(width);
			loadedImage.Height = uint32_t(height);
			loadedImage.Data.resize(width * height * 4);
			memcpy(loadedImage.Data.data(), imageData, width * height * 4);

			stbi_image_free(imageData);
			YUKI_STOPWATCH_STOP();
		}

		YUKI_STOPWATCH_START_N("Process Materials");
		for (auto& material : InAsset->materials)
		{
			auto& meshMaterial = InMeshData.Materials.emplace_back();

			if (!material.pbrData.has_value())
				continue;

			auto& pbrData = material.pbrData.value();

			if (!pbrData.baseColorTexture.has_value())
				continue;

			auto& colorTexture = pbrData.baseColorTexture.value();
			meshMaterial.AlbedoTextureIndex = uint32_t(colorTexture.textureIndex);
		}
		YUKI_STOPWATCH_STOP();
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

		YUKI_STOPWATCH_START_N("Load GLTF File");
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
		YUKI_STOPWATCH_STOP();

		YUKI_VERIFY(parser.getError() == fastgltf::Error::None);
		
		YUKI_STOPWATCH_START_N("Parse GLTF File");
		YUKI_VERIFY(gltfAsset->parse() == fastgltf::Error::None);
		YUKI_STOPWATCH_STOP();

		auto asset = gltfAsset->getParsedAsset();

		LoadedMesh result = {};
		result.Meshes.reserve(asset->meshes.size());

		ProcessMaterials(InContext, asset.get(), InFilePath.parent_path(), result);

		YUKI_STOPWATCH_START_N("Process Vertex Data");
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
						meshData.Vertices[vertexID].Position = InPosition;
						meshData.Vertices[vertexID].MaterialIndex = uint32_t(primitive.materialIndex.value_or(0));
						vertexID++;
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
		}
		YUKI_STOPWATCH_STOP();

		fastgltf::Scene* scene = nullptr;
		if (asset->defaultScene.has_value())
			scene = &asset->scenes[asset->defaultScene.value()];
		else
			scene = &asset->scenes[0];
		YUKI_VERIFY(scene);

		result.Instances.reserve(scene->nodeIndices.size());

		Math::Mat4 transform;
		transform.SetIdentity();

		YUKI_STOPWATCH_START_N("Process Node Hierarchy");
		for (auto nodeIndex : scene->nodeIndices)
			ProcessNodeHierarchy(asset.get(), result, asset->nodes[nodeIndex], transform);
		YUKI_STOPWATCH_STOP();

		return result;
	}
}
