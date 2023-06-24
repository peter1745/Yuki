#include "IO/MeshLoader.hpp"
#include "Core/Timer.hpp"

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

	static constexpr uint32_t s_StagingBufferSize = 100 * 1024 * 1024;

	void ProcessNodeHierarchy(fastgltf::Asset* InAsset, Mesh& InMesh, fastgltf::Node& InNode, const Math::Mat4& InParentTransform)
	{
		auto& TRS = std::get<fastgltf::Node::TRS>(InNode.transform);
		Math::Mat4 modelTransform = Math::Mat4::Translation(Math::Vec3{TRS.translation}) * Math::Mat4::Rotation(Math::Quat{ TRS.rotation }) * Math::Mat4::Scale(Math::Vec3{TRS.scale});
		Math::Mat4 transform = InParentTransform * modelTransform;

		if (InNode.meshIndex.has_value())
		{
			auto& meshInstance = InMesh.Instances.emplace_back();
			meshInstance.SourceIndex = InNode.meshIndex.value();
			meshInstance.Transform = transform;
		}

		for (auto childNodeIndex : InNode.children)
			ProcessNodeHierarchy(InAsset, InMesh, InAsset->nodes[childNodeIndex], transform);
	}

#if 0
	void ProcessMaterials(fastgltf::Asset* InAsset, const std::filesystem::path& InBasePath, LoadedMesh& InMeshData)
	{
		ScopedTimer timer("Process Materials");
		InMeshData.LoadedImages.resize(InAsset->textures.size());

#pragma omp parallel for
		for (size_t i = 0; i < InAsset->textures.size(); i++)
		{
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
		}

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
	}
#endif

	void FlushStagingBuffer(RenderContext* InContext, CommandPool InCommandPool, CommandList InCommandList)
	{
		InContext->CommandListEnd(InCommandList);
		InContext->QueueSubmitCommandLists(InContext->GetTransferQueue(), { InCommandList }, {}, {});
		InContext->QueueWaitIdle(InContext->GetTransferQueue());

		InContext->CommandPoolReset(InCommandPool);
		InContext->CommandListBegin(InCommandList);
	}

	MeshLoader::MeshLoader(RenderContext* InContext, PushMeshCallback InCallback)
		: m_Context(InContext), m_Callback(std::move(InCallback))
	{
		const uint32_t jobCount = 3;
		m_JobSystem.Init(jobCount);

		m_CommandPool = m_Context->CreateCommandPool(m_Context->GetTransferQueue());
		m_StagingBuffer = m_Context->CreateBuffer({
			.Type = BufferType::StagingBuffer,
			.Size = s_StagingBufferSize
		});

		m_JobSystem.Submit([this](size_t InWorkerID)
		{
			std::scoped_lock lock(m_UploadQueueMutex);

			if (m_UploadQueue.empty())
				return;

			LogInfo("Running Upload Job, upload queue size: {}", m_UploadQueue.size());

			m_Context->CommandPoolReset(m_CommandPool);

			for (auto&[mesh, meshes] : m_UploadQueue)
			{
				DynamicArray<std::pair<Buffer, Buffer>> buffers;

				auto commandList = m_Context->CreateCommandList(m_CommandPool);
				m_Context->CommandListBegin(commandList);

				uint32_t offset = 0;
				for (const auto& meshData : meshes)
				{
					Buffer vertexBuffer;
					Buffer indexBuffer;

					{
						uint32_t vertexDataSize = uint32_t(meshData.Vertices.size()) * sizeof(Vertex);

						if (s_StagingBufferSize - offset < vertexDataSize)
						{
							FlushStagingBuffer(m_Context, m_CommandPool, commandList);
							offset = 0;
						}

						m_Context->BufferSetData(m_StagingBuffer, meshData.Vertices.data(), vertexDataSize, offset);

						vertexBuffer = m_Context->CreateBuffer({
							.Type = BufferType::StorageBuffer,
							.Size = vertexDataSize
						});

						m_Context->CommandListCopyToBuffer(commandList, vertexBuffer, 0, m_StagingBuffer, offset, 0);
						offset += vertexDataSize;
					}

					{
						uint32_t indexDataSize = uint32_t(meshData.Indices.size()) * sizeof(uint32_t);

						if (s_StagingBufferSize - offset < indexDataSize)
						{
							FlushStagingBuffer(m_Context, m_CommandPool, commandList);
							offset = 0;
						}

						m_Context->BufferSetData(m_StagingBuffer, meshData.Indices.data(), indexDataSize, offset);

						indexBuffer = m_Context->CreateBuffer({
							.Type = BufferType::IndexBuffer,
							.Size = indexDataSize
						});

						m_Context->CommandListCopyToBuffer(commandList, indexBuffer, 0, m_StagingBuffer, offset, 0);
						offset += indexDataSize;
					}

					buffers.push_back({ vertexBuffer, indexBuffer });
				}

				m_Context->CommandListEnd(commandList);
				m_Context->QueueSubmitCommandLists(m_Context->GetTransferQueue(), { commandList }, {}, {});
				m_Context->QueueWaitIdle(m_Context->GetTransferQueue());

				mesh.Sources.resize(buffers.size());
				for (size_t i = 0; i < buffers.size(); i++)
				{
					mesh.Sources[i] =
					{
						.VertexData = buffers[i].first,
						.IndexBuffer = buffers[i].second,
						.IndexCount = uint32_t(meshes[i].Indices.size())
					};
				}

				m_Callback(mesh);
			}

			m_UploadQueue.clear();
			LogInfo("Upload Job Finished, upload queue size: {}", m_UploadQueue.size());
			
		}, JobFlags::RescheduleOnFinish);
	}

	void MeshLoader::LoadGLTFMesh(const std::filesystem::path& InFilePath)
	{
		m_JobSystem.Submit([this, filePath = InFilePath](size_t InWorkerID)
		{
			fastgltf::Parser parser;
			fastgltf::GltfDataBuffer dataBuffer;
			dataBuffer.loadFromFile(filePath);

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
				gltfAsset = parser.loadGLTF(&dataBuffer, filePath.parent_path(), options);
				break;
			}
			case fastgltf::GltfType::GLB:
			{
				gltfAsset = parser.loadBinaryGLTF(&dataBuffer, filePath.parent_path(), options);
				break;
			}
			}

			YUKI_VERIFY(parser.getError() == fastgltf::Error::None);
			YUKI_VERIFY(gltfAsset->parse() == fastgltf::Error::None);

			auto asset = gltfAsset->getParsedAsset();

			DynamicArray<MeshData> meshes;
			meshes.reserve(asset->meshes.size());

			//ProcessMaterials(asset.get(), InFilePath.parent_path(), result);

			for (auto& mesh : asset->meshes)
			{
				MeshData& meshData = meshes.emplace_back();

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


			//LogInfo("Creating buffers for {} meshes", meshes.size());
			//auto buffers = CreateGPUBuffers(meshes);
			//LogInfo("Created {} buffers", buffers.size() * 2);

			Mesh result;

			fastgltf::Scene* scene = nullptr;
			if (asset->defaultScene.has_value())
				scene = &asset->scenes[asset->defaultScene.value()];
			else
				scene = &asset->scenes[0];
			YUKI_VERIFY(scene);

			result.Instances.reserve(scene->nodeIndices.size());

			Math::Mat4 transform;
			transform.SetIdentity();

			for (auto nodeIndex : scene->nodeIndices)
				ProcessNodeHierarchy(asset.get(), result, asset->nodes[nodeIndex], transform);

			std::scoped_lock lock(m_UploadQueueMutex);
			LogInfo("Scheduling Mesh for GPU Upload, upload queue size: {}", m_UploadQueue.size());
			m_UploadQueue.push_back({ result, meshes });
			//m_Callback(std::move(result));
		});
	}

#if 0
	DynamicArray<std::pair<Buffer, Buffer>> MeshLoader::CreateGPUBuffers(const DynamicArray<MeshData>& InMeshes)
	{
		DynamicArray<std::pair<Buffer, Buffer>> result;

		m_Context->CommandListBegin(m_CommandList);

		uint32_t offset = 0;
		for (const auto& meshData : InMeshes)
		{
			Buffer vertexBuffer;
			Buffer indexBuffer;

			{
				uint32_t vertexDataSize = uint32_t(meshData.Vertices.size()) * sizeof(Vertex);

				if (s_StagingBufferSize - offset < vertexDataSize)
				{
					FlushStagingBuffer();
					offset = 0;
				}

				m_Context->BufferSetData(m_StagingBuffer, meshData.Vertices.data(), vertexDataSize, offset);

				vertexBuffer = m_Context->CreateBuffer({
					.Type = BufferType::StorageBuffer,
					.Size = vertexDataSize
				});

				m_Context->CommandListCopyToBuffer(m_CommandList, vertexBuffer, 0, m_StagingBuffer, offset, 0);
				offset += vertexDataSize;
			}

			{
				uint32_t indexDataSize = uint32_t(meshData.Indices.size()) * sizeof(uint32_t);

				if (s_StagingBufferSize - offset < indexDataSize)
				{
					FlushStagingBuffer();
					offset = 0;
				}

				m_Context->BufferSetData(m_StagingBuffer, meshData.Indices.data(), indexDataSize, offset);

				indexBuffer = m_Context->CreateBuffer({
					.Type = BufferType::IndexBuffer,
					.Size = indexDataSize
				});

				m_Context->CommandListCopyToBuffer(m_CommandList, indexBuffer, 0, m_StagingBuffer, offset, 0);
				offset += indexDataSize;
			}

			result.push_back({ vertexBuffer, indexBuffer });
		}

		m_Context->CommandListEnd(m_CommandList);
		m_Context->QueueSubmitCommandLists(m_Context->GetTransferQueue(), { m_CommandList }, {}, {});
		m_Context->QueueWaitIdle(m_Context->GetTransferQueue());

		return result;
	}
#endif

}

