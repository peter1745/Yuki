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
		Logging::Info("Loaded {} textures", model.Textures.size());

		model.Materials.resize(asset->materials.size());

		//#pragma omp parallel for
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

			Logging::Info("Material: {}, Index: {}", gltfMaterial.name, i);

			switch (gltfMaterial.alphaMode)
			{
			case fastgltf::AlphaMode::Opaque:
				Logging::Info("\tAlpha Mode: Opaque");
				break;
			case fastgltf::AlphaMode::Mask:
				Logging::Info("\tAlpha Mode: Mask");
				break;
			case fastgltf::AlphaMode::Blend:
				Logging::Info("\tAlpha Mode: Blend");
				material.AlphaBlending = 1;
				break;
			default:
				break;
			}
		}
		Logging::Info("Loaded {} materials", model.Materials.size());

		DynamicArray<std::pair<uint32_t, uint32_t>> meshOffsets(asset->meshes.size());

		for (uint32_t i = 0; i < asset->meshes.size(); i++)
		{
			const auto& gltfMesh = asset->meshes[i];

			meshOffsets[i].first = model.Meshes.size();
			meshOffsets[i].second = gltfMesh.primitives.size();

			Logging::Info("Mesh: {}", gltfMesh.name);

			for (const auto& primitive : gltfMesh.primitives)
			{
				Logging::Info("Primitive: {}", model.Meshes.size());
				const auto positionAttrib = primitive.findAttribute("POSITION");

				if (positionAttrib == primitive.attributes.end())
					break;

				if (!primitive.indicesAccessor)
					break;

				auto& meshData = model.Meshes.emplace_back();

				const auto& positionAccessor = asset->accessors[positionAttrib->second];
				const auto& indicesAccessor = asset->accessors[primitive.indicesAccessor.value()];

				size_t vertexID = 0;

				meshData.Indices.resize(indicesAccessor.count);
				meshData.Positions.resize(positionAccessor.count);
				meshData.ShadingAttributes.resize(positionAccessor.count);

				fastgltf::iterateAccessorWithIndex<uint32_t>(asset, indicesAccessor, [&](uint32_t vertexIndex, size_t i)
				{
					meshData.Indices[i] = vertexIndex;
				});

				fastgltf::iterateAccessor<Vec3>(asset, positionAccessor, [&](const Vec3& position)
				{
					meshData.Positions[vertexID++] = position;
				});
				vertexID = 0;

				const auto normalsAttrib = primitive.findAttribute("NORMAL");
				if (normalsAttrib != primitive.attributes.end())
				{
					const auto& normalsAccessor = asset->accessors[normalsAttrib->second];
					fastgltf::iterateAccessor<Vec3>(asset, normalsAccessor, [&](const Vec3& normal)
					{
						meshData.ShadingAttributes[vertexID++].Normal = normal;
					});
					vertexID = 0;
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

				if (primitive.materialIndex)
				{
					meshData.MaterialIndex = primitive.materialIndex.value_or(0);
					Logging::Info("\tMaterial: {}", meshData.MaterialIndex);
				}
			}
		}

		Logging::Info("[glTF]: Loaded {} meshes from {}", model.Meshes.size(), filepath.string());

		auto processNode = [&](this auto&& self, MeshScene& scene, const fastgltf::Node& gltfNode, const Mat4& parentTransform) -> void
		{
			auto TRS = std::get<fastgltf::Node::TRS>(gltfNode.transform);

			Mat4 transform =
				parentTransform *
				(glm::translate(Vec3{ TRS.translation[0], TRS.translation[1], TRS.translation[2] }) *
				glm::mat4_cast(Quat{ TRS.rotation[3], TRS.rotation[0], TRS.rotation[1], TRS.rotation[2] }) *
				glm::scale(Vec3{TRS.scale[0], TRS.scale[1], TRS.scale[2] }));

			if (gltfNode.meshIndex)
			{
				auto [meshIndex, meshCount] = meshOffsets[gltfNode.meshIndex.value()];
				for (uint32_t i = 0; i < meshCount; i++)
				{
					auto& instance = scene.Instances.emplace_back();
					instance.Name = std::string(gltfNode.name);
					instance.Transform = transform;
					instance.MeshIndex = meshIndex + i;
				}
			}

			for (auto childIndex : gltfNode.children)
			{
				self(scene, asset->nodes[childIndex], transform);
			}
		};

		for (const auto& gltfScene : asset->scenes)
		{
			auto& scene = model.Scenes.emplace_back();

			for (auto nodeIndex : gltfScene.nodeIndices)
			{
				processNode(scene, asset->nodes[nodeIndex], Mat4(1.0f));
			}
		}
	}

}
