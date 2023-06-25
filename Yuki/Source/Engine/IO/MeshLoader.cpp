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

	void MeshLoader::ProcessMaterials(fastgltf::Asset* InAsset, const std::filesystem::path& InBasePath, MeshData& InMeshData)
	{
		InMeshData.Images.resize(InAsset->textures.size());
		LogInfo("Loading {} textures...", InAsset->textures.size());

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

			auto& image = InMeshData.Images[i];
			image.Width = uint32_t(width);
			image.Height = uint32_t(height);
			image.Data.resize(width * height * 4);
			memcpy(image.Data.data(), imageData, width * height * 4);

			stbi_image_free(imageData);
		}
	}

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
		m_JobSystem.Init(3);

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

			for (auto&[mesh, meshData] : m_UploadQueue)
			{
				DynamicArray<std::pair<Buffer, Buffer>> buffers;
				
				m_Context->CommandPoolReset(m_CommandPool);
				auto commandList = m_Context->CreateCommandList(m_CommandPool);
				m_Context->CommandListBegin(commandList);
				for (const auto& instanceData : meshData.InstanceData)
				{
					Buffer vertexBuffer;
					Buffer indexBuffer;

					{
						uint32_t vertexDataSize = uint32_t(instanceData.Vertices.size()) * sizeof(Vertex);

						vertexBuffer = m_Context->CreateBuffer({
							.Type = BufferType::StorageBuffer,
							.Size = vertexDataSize
						});

						if (vertexBuffer != Buffer{})
						{
							for (uint32_t bufferOffset = 0; bufferOffset < vertexDataSize; bufferOffset += s_StagingBufferSize)
							{
								const std::byte* ptr = (const std::byte*)instanceData.Vertices.data();
								uint32_t blockSize = std::min(bufferOffset + s_StagingBufferSize, vertexDataSize) - bufferOffset;
								m_Context->BufferSetData(m_StagingBuffer, ptr + bufferOffset, blockSize, 0);
								m_Context->CommandListCopyToBuffer(commandList, vertexBuffer, bufferOffset, m_StagingBuffer, 0, blockSize);
								FlushStagingBuffer(m_Context, m_CommandPool, commandList);
							}
						}
					}

					{
						uint32_t indexDataSize = uint32_t(instanceData.Indices.size()) * sizeof(uint32_t);

						indexBuffer = m_Context->CreateBuffer({
							.Type = BufferType::IndexBuffer,
							.Size = indexDataSize
						});

						if (indexBuffer != Buffer{})
						{
							for (uint32_t bufferOffset = 0; bufferOffset < indexDataSize; bufferOffset += s_StagingBufferSize)
							{
								const std::byte* ptr = (const std::byte*)instanceData.Indices.data();
								uint32_t blockSize = std::min(bufferOffset + s_StagingBufferSize, indexDataSize) - bufferOffset;
								m_Context->BufferSetData(m_StagingBuffer, ptr + bufferOffset, blockSize, 0);
								m_Context->CommandListCopyToBuffer(commandList, indexBuffer, bufferOffset, m_StagingBuffer, 0, blockSize);
								FlushStagingBuffer(m_Context, m_CommandPool, commandList);
							}
						}
					}

					buffers.push_back({ vertexBuffer, indexBuffer });
				}

				m_Context->CommandListEnd(commandList);
				m_Context->QueueSubmitCommandLists(m_Context->GetTransferQueue(), { commandList }, {}, {});
				m_Context->QueueWaitIdle(m_Context->GetTransferQueue());

				m_Context->CommandPoolReset(m_CommandPool);

				mesh.Sources.resize(meshData.InstanceData.size());
				for (size_t i = 0; i < meshData.InstanceData.size(); i++)
				{
					mesh.Sources[i] =
					{
						.VertexData = buffers[i].first,
						.IndexBuffer = buffers[i].second,
						.IndexCount = uint32_t(meshData.InstanceData[i].Indices.size())
					};
				}

				{
					mesh.Textures.resize(meshData.Images.size());
					for (size_t i = 0; i < meshData.Images.size(); i++)
					{
						const auto& imageData = meshData.Images[i];
						bool shouldBlit = imageData.Width > 1024 && imageData.Height > 1024;
						Queue queue = shouldBlit ? m_Context->GetGraphicsQueue(1) : m_Context->GetTransferQueue();

						auto commandPool = m_Context->CreateCommandPool(queue);
						auto imageCommandList = m_Context->CreateCommandList(commandPool);
						m_Context->CommandListBegin(imageCommandList);

						Image blittedImage{};
						Image image = m_Context->CreateImage(imageData.Width, imageData.Height, ImageFormat::RGBA8UNorm, ImageUsage::Sampled | ImageUsage::TransferSource | Yuki::ImageUsage::TransferDestination);
						m_Context->CommandListTransitionImage(imageCommandList, image, ImageLayout::ShaderReadOnly);
						m_Context->BufferSetData(m_StagingBuffer, imageData.Data.data(), uint32_t(imageData.Data.size()));
						m_Context->CommandListCopyToImage(imageCommandList, image, m_StagingBuffer, 0);

						if (shouldBlit)
						{
							blittedImage = m_Context->CreateImage(1024, 1024, ImageFormat::RGBA8UNorm, ImageUsage::Sampled | ImageUsage::TransferDestination);
							m_Context->CommandListTransitionImage(imageCommandList, blittedImage, ImageLayout::ShaderReadOnly);
							m_Context->CommandListBlitImage(imageCommandList, blittedImage, image);
						}

						m_Context->CommandListEnd(imageCommandList);
						m_Context->QueueSubmitCommandLists(queue, { imageCommandList }, {}, {});
						m_Context->QueueWaitIdle(queue);

						if (shouldBlit)
						{
							m_Context->Destroy(image);
							image = blittedImage;
						}

						mesh.Textures[i] = image;
					}
				}

				//m_Context->CommandPoolReset(m_CommandPool);

				/*{
					mesh.MaterialsBuffer = m_Context->CreateBuffer({
						.Type = BufferType::StorageBuffer,
						.Size = uint32_t(sizeof(Yuki::MeshMaterial) * meshData.Materials.size())
					});

					auto materialsCommandList = m_Context->CreateCommandList(m_CommandPool);
					m_Context->CommandListBegin(materialsCommandList);
					m_Context->BufferSetData(m_StagingBuffer, meshData.Materials.data(), uint32_t(meshData.Materials.size() * sizeof(MeshMaterial)));
					m_Context->CommandListCopyToBuffer(materialsCommandList, mesh.MaterialsBuffer, 0, m_StagingBuffer, 0, 0);
					m_Context->CommandListEnd(materialsCommandList);
					m_Context->QueueSubmitCommandLists(m_Context->GetTransferQueue(), { materialsCommandList }, {}, {});
					m_Context->QueueWaitIdle(m_Context->GetTransferQueue());
				}*/

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

			std::scoped_lock lock(m_UploadQueueMutex);
			auto&[mesh, meshData] = m_UploadQueue.emplace_back();
			meshData.InstanceData.reserve(asset->meshes.size());

			ProcessMaterials(asset.get(), filePath.parent_path(), meshData);

			LogInfo("Done! Loading {} materials...", asset->materials.size());

			for (auto& material : asset->materials)
			{
				auto& meshMaterial = mesh.Materials.emplace_back();

				if (!material.pbrData.has_value())
					continue;

				auto& pbrData = material.pbrData.value();

				if (!pbrData.baseColorTexture.has_value())
					continue;

				auto& colorTexture = pbrData.baseColorTexture.value();
				meshMaterial.AlbedoTextureIndex = uint32_t(colorTexture.textureIndex);
			}

			LogInfo("Done!");

			for (auto& gltfMesh : asset->meshes)
			{
				MeshInstanceData& instanceData = meshData.InstanceData.emplace_back();

				for (auto& primitive : gltfMesh.primitives)
				{
					if (primitive.attributes.find("POSITION") == primitive.attributes.end())
						continue;

					auto& positionAccessor = asset->accessors[primitive.attributes["POSITION"]];

					if (!primitive.indicesAccessor.has_value())
						break;

					size_t baseVertexOffset = instanceData.Vertices.size();
					size_t vertexID = baseVertexOffset;
					instanceData.Vertices.resize(baseVertexOffset + positionAccessor.count);

					{
						auto& indicesAccessor = asset->accessors[primitive.indicesAccessor.value()];
						fastgltf::iterateAccessor<uint32_t>(*asset, indicesAccessor, [&](uint32_t InIndex)
						{
							instanceData.Indices.emplace_back(InIndex + uint32_t(baseVertexOffset));
						});
					}

					{
						fastgltf::iterateAccessor<Math::Vec3>(*asset, positionAccessor, [&](Math::Vec3 InPosition)
						{
							instanceData.Vertices[vertexID].Position = InPosition;
							instanceData.Vertices[vertexID].MaterialIndex = uint32_t(primitive.materialIndex.value_or(0));
							vertexID++;
						});
						vertexID = baseVertexOffset;
					}

					if (primitive.attributes.contains("NORMAL"))
					{
						auto& normalsAccessor = asset->accessors[primitive.attributes["NORMAL"]];
						fastgltf::iterateAccessor<Math::Vec3>(*asset, normalsAccessor, [&](Math::Vec3 InNormal)
						{
							instanceData.Vertices[vertexID++].Normal = InNormal;
						});
						vertexID = baseVertexOffset;
					}

					if (primitive.attributes.contains("TEXCOORD_0"))
					{
						auto& uvAccessor = asset->accessors[primitive.attributes["TEXCOORD_0"]];
						fastgltf::iterateAccessor<Math::Vec2>(*asset, uvAccessor, [&](Math::Vec2 InUV)
						{
							instanceData.Vertices[vertexID++].UV = InUV;
						});
						vertexID = baseVertexOffset;
					}
				}
			}

			fastgltf::Scene* scene = nullptr;
			if (asset->defaultScene.has_value())
				scene = &asset->scenes[asset->defaultScene.value()];
			else
				scene = &asset->scenes[0];
			YUKI_VERIFY(scene);

			mesh.Instances.reserve(scene->nodeIndices.size());

			Math::Mat4 transform;
			transform.SetIdentity();

			for (auto nodeIndex : scene->nodeIndices)
				ProcessNodeHierarchy(asset.get(), mesh, asset->nodes[nodeIndex], transform);
		});
	}
}

