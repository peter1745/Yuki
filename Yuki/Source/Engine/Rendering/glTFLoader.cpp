#include "glTFLoader.hpp"

#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include <stb_image/stb_image.h>

namespace Yuki {

	void glTFLoader::Load(const std::filesystem::path& filepath, Model& model)
	{
		fastgltf::Parser parser;
		fastgltf::GltfDataBuffer dataBuffer;
		
		if (!dataBuffer.loadFromFile(filepath))
		{
			Logging::Error("Failed to load glTF file: {}", filepath.string());
			return;
		}

		fastgltf::Options options =
			fastgltf::Options::DontRequireValidAssetMember |
			fastgltf::Options::LoadGLBBuffers |
			fastgltf::Options::LoadExternalBuffers |
			fastgltf::Options::DecomposeNodeMatrices;

		Unique<fastgltf::Asset> asset = nullptr;

		{
			fastgltf::Expected<fastgltf::Asset> expectedAsset{ fastgltf::Error::None };

			switch (fastgltf::determineGltfFileType(&dataBuffer))
			{
			case fastgltf::GltfType::glTF:
			{
				expectedAsset = parser.loadGLTF(&dataBuffer, filepath.parent_path(), options);
				break;
			}
			case fastgltf::GltfType::GLB:
				expectedAsset = parser.loadBinaryGLTF(&dataBuffer, filepath.parent_path(), options);
				break;
			default:
				YUKI_VERIFY(false, "Invalid glTF Type!");
				break;
			}

			if (expectedAsset.error() != fastgltf::Error::None)
			{
				Logging::Error("Failed to load {}. Error: {}", filepath.string(), fastgltf::getErrorMessage(expectedAsset.error()));
				return;
			}

			asset = Unique<fastgltf::Asset>::New(std::move(expectedAsset.get()));
		}

		model.Textures.resize(asset->images.size());

		#pragma omp parallel for
		for (size_t i = 0; i < asset->images.size(); i++)
		{
			const auto& gltfImage = asset->images[i];

			auto& texture = model.Textures[i];

			auto createFromBytes = [&](const std::byte* data, int32_t dataSize)
			{
				int32_t width = 0;
				int32_t height = 0;
				auto* imageData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(data), dataSize, &width, &height, nullptr, STBI_rgb_alpha);

				size_t imageSize = Cast<size_t>(width * height * 4);
				texture.Width = Cast<uint32_t>(width);
				texture.Height = Cast<uint32_t>(height);
				texture.Data.assign(reinterpret_cast<std::byte*>(imageData), reinterpret_cast<std::byte*>(imageData) + imageSize);

				stbi_image_free(imageData);
			};

			std::visit(fastgltf::visitor
			{
				[&](const fastgltf::sources::URI& uri)
				{
					Logging::Info("Loading texture from URI");
					auto fp = (filepath.parent_path() / uri.uri.path()).string();
					int32_t width = 0;
					int32_t height = 0;
					auto* imageData = stbi_load(fp.c_str(), &width, &height, nullptr, STBI_rgb_alpha);

					size_t imageSize = Cast<size_t>(width * height * 4);
					texture.Width = Cast<uint32_t>(width);
					texture.Height = Cast<uint32_t>(height);
					texture.Data.assign(reinterpret_cast<std::byte*>(imageData), reinterpret_cast<std::byte*>(imageData) + imageSize);

					stbi_image_free(imageData);
				},
				[&](const fastgltf::sources::Vector& vector)
				{
					Logging::Info("Loading texture from Vector");
					createFromBytes(reinterpret_cast<const std::byte*>(vector.bytes.data()), Cast<int32_t>(vector.bytes.size()));
				},
				[&](const fastgltf::sources::ByteView& byteView)
				{
					Logging::Info("Loading texture from ByteView");
					createFromBytes(byteView.bytes.data(), Cast<int32_t>(byteView.bytes.size()));
				},
				[&](const fastgltf::sources::BufferView& bufferView)
				{
					Logging::Info("Loading texture from BufferView");
					const auto& view = asset->bufferViews[bufferView.bufferViewIndex];
					const auto& buffer = asset->buffers[view.bufferIndex];
					const auto* bytes = fastgltf::DefaultBufferDataAdapter{}(buffer) + view.byteOffset;
					createFromBytes(bytes, Cast<int32_t>(view.byteLength));
				},
				[&](auto&&)
				{
					YUKI_VERIFY(false);
				}
			}, gltfImage.data);
		}

		model.Materials.resize(asset->materials.size());

		#pragma omp parallel for
		for (size_t i = 0; i < asset->materials.size(); i++)
		{
			const auto& gltfMaterial = asset->materials[i];
			auto& material = model.Materials[i];

			const auto& c = gltfMaterial.pbrData.baseColorFactor;
			material.BaseColor = glm::packUnorm4x8({ c[0], c[1], c[2], c[3] });

			if (gltfMaterial.pbrData.baseColorTexture)
			{
				const auto& texture = gltfMaterial.pbrData.baseColorTexture.value();
				material.BaseColorTextureIndex = Cast<int32_t>(asset->textures[texture.textureIndex].imageIndex.value());
			}
		}

		for (const auto& gltfMesh : asset->meshes)
		{
			auto& meshData = model.Meshes.emplace_back();

			for (const auto& primitive : gltfMesh.primitives)
			{
				const auto positionAttrib = primitive.findAttribute("POSITION");

				if (positionAttrib == primitive.attributes.end())
					break;

				if (!primitive.indicesAccessor)
					break;

				const auto& positionAccessor = asset->accessors[positionAttrib->second];
				const auto& indicesAccessor = asset->accessors[primitive.indicesAccessor.value()];

				size_t baseVertex = meshData.Positions.size();
				size_t baseIndex = meshData.Indices.size();
				size_t vertexID = baseVertex;

				meshData.Indices.resize(baseIndex + indicesAccessor.count);
				meshData.Positions.resize(baseVertex + positionAccessor.count);
				meshData.ShadingAttributes.resize(baseVertex + positionAccessor.count);

				fastgltf::iterateAccessorWithIndex<uint32_t>(asset, indicesAccessor, [&](uint32_t vertexIndex, size_t i)
				{
					meshData.Indices[baseIndex + i] = vertexIndex + Cast<uint32_t>(baseVertex);
				});

				fastgltf::iterateAccessor<Vec3>(asset, positionAccessor, [&](const Vec3& position)
				{
					meshData.Positions[vertexID] = position;
					meshData.ShadingAttributes[vertexID].MaterialIndex = Cast<uint32_t>(primitive.materialIndex.value_or(0));
					vertexID++;
				});
				vertexID = baseVertex;

				const auto normalsAttrib = primitive.findAttribute("NORMAL");
				if (normalsAttrib != primitive.attributes.end())
				{
					const auto& normalsAccessor = asset->accessors[normalsAttrib->second];
					fastgltf::iterateAccessor<Vec3>(asset, normalsAccessor, [&](const Vec3& normal)
					{
						meshData.ShadingAttributes[vertexID++].Normal = normal;
					});
					vertexID = baseVertex;
				}

				const auto texCoordsAttrib = primitive.findAttribute("TEXCOORD_0");
				if (texCoordsAttrib != primitive.attributes.end())
				{
					const auto& texCoordsAccessor = asset->accessors[texCoordsAttrib->second];
					fastgltf::iterateAccessor<Vec2>(asset, texCoordsAccessor, [&](const Vec2& texCoord)
					{
						meshData.ShadingAttributes[vertexID++].TexCoord = texCoord;
					});
				}
			}
		}

		Logging::Info("[glTF]: Loaded {} meshes from {}", model.Meshes.size(), filepath.string());

		auto processNode = [&](const fastgltf::Node& gltfNode)
		{
			auto TRS = std::get<fastgltf::Node::TRS>(gltfNode.transform);

			auto& node = model.Nodes.emplace_back();
			node.Name = std::string(gltfNode.name);
			node.Translation = { TRS.translation[0], TRS.translation[1], TRS.translation[2] };
			node.Rotation = { TRS.rotation[3], TRS.rotation[0], TRS.rotation[1], TRS.rotation[2] };
			node.Scale = { TRS.scale[0], TRS.scale[1], TRS.scale[2] };
			node.MeshIndex = gltfNode.meshIndex.has_value() ? Cast<int32_t>(gltfNode.meshIndex.value()) : -1;
			Logging::Info("Node: {}, MeshIndex: {}", node.Name, node.MeshIndex);

			for (auto childIndex : gltfNode.children)
				node.ChildNodes.push_back(childIndex);
		};

		for (const auto& node : asset->nodes)
			processNode(node);

		for (const auto& gltfScene : asset->scenes)
		{
			auto& scene = model.Scenes.emplace_back();

			for (auto nodeIndex : gltfScene.nodeIndices)
				scene.NodeIndices.push_back(nodeIndex);
		}
	}

}
