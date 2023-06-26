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

	void ProcessNodeHierarchy(fastgltf::Asset* InAsset, Mesh& InMesh, size_t InNodeIndex, const Math::Mat4& InParentTransform)
	{
		auto& node = InAsset->nodes[InNodeIndex];
		auto& TRS = std::get<fastgltf::Node::TRS>(node.transform);
		Math::Mat4 modelTransform = Math::Mat4::Translation(Math::Vec3{TRS.translation}) * Math::Mat4::Rotation(Math::Quat{ TRS.rotation }) * Math::Mat4::Scale(Math::Vec3{TRS.scale});
		Math::Mat4 transform = InParentTransform * modelTransform;

		if (node.meshIndex.has_value())
		{
			size_t sourceIndex = node.meshIndex.value();
			const auto& sourceMesh = InMesh.Sources[sourceIndex];
			if (sourceMesh.VertexData != Buffer{} && sourceMesh.IndexBuffer != Buffer{} && sourceMesh.IndexCount > 0)
			{
				auto& meshInstance = InMesh.Instances[InNodeIndex];
				meshInstance.SourceIndex = sourceIndex;
				meshInstance.Transform = transform;
			}
		}

		for (auto childNodeIndex : node.children)
			ProcessNodeHierarchy(InAsset, InMesh, childNodeIndex, transform);
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
		m_JobSystem.Init(8);

		m_MeshStagingBuffer = m_Context->CreateBuffer({
			.Type = BufferType::StagingBuffer,
			.Size = s_StagingBufferSize
		});

		m_ImageStagingBuffer = m_Context->CreateBuffer({
			.Type = BufferType::StagingBuffer,
			.Size = s_StagingBufferSize
		});
	}

	void MeshLoader::LoadGLTFMesh(const std::filesystem::path& InFilePath)
	{
		fastgltf::Parser parser;
		fastgltf::GltfDataBuffer dataBuffer;
		dataBuffer.loadFromFile(InFilePath);

		std::unique_ptr<fastgltf::glTF> gltfAsset;

		fastgltf::Options options = fastgltf::Options::DontRequireValidAssetMember |
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
		YUKI_VERIFY(gltfAsset->parse() == fastgltf::Error::None);

		auto* assetPtr = gltfAsset->getParsedAsset().release();

		auto[meshIndex, mesh] = m_MeshQueue.EmplaceBack();

		auto[meshDataIndex, meshData] = m_ProcessingQueue.EmplaceBack();
		mesh.Sources.resize(assetPtr->meshes.size());
		meshData.SourceData.resize(assetPtr->meshes.size());
		meshData.Images.resize(assetPtr->textures.size());
		mesh.Textures.resize(assetPtr->textures.size());

		fastgltf::Scene* scene = nullptr;
		if (assetPtr->defaultScene.has_value())
			scene = &assetPtr->scenes[assetPtr->defaultScene.value()];
		else
			scene = &assetPtr->scenes[0];
		YUKI_VERIFY(scene);

		mesh.Instances.resize(assetPtr->nodes.size());

		auto[instanceCreateBarrierIndex, instanceCreateBarrier] = m_Barriers.EmplaceBack();

#define SINGLE_IMAGE_JOB 0
#if SINGLE_IMAGE_JOB
		auto[barrierIndex0, imageUploadBarrier] = m_Barriers.EmplaceBack();
		auto[index, imageLoadJob] = m_LoadJobs.EmplaceBack();
		imageLoadJob.Task = [asset = assetPtr, &mesh, &meshData, basePath = InFilePath.parent_path()](size_t InThreadID)
		{
			for (size_t i = 0; i < asset->textures.size(); i++)
			{
				auto& textureInfo = asset->textures[i];

				YUKI_VERIFY(textureInfo.imageIndex.has_value());
				auto& imageInfo = asset->images[textureInfo.imageIndex.value()];

				int width, height;
				stbi_uc* imageData = nullptr;
				std::visit(ImageVisitor
				{
					[&](fastgltf::sources::URI& InURI)
					{
						ScopedTimer t("URI");
						imageData = stbi_load(fmt::format("{}/{}", basePath.string(), InURI.uri.path()).c_str(), &width, &height, nullptr, STBI_rgb_alpha);
					},
					[&](fastgltf::sources::Vector& InVector)
					{
						ScopedTimer t("Vector");
						imageData = stbi_load_from_memory(InVector.bytes.data(), uint32_t(InVector.bytes.size()), &width, &height, nullptr, STBI_rgb_alpha);
					},
					[&](fastgltf::sources::ByteView& InByteView)
					{
						ScopedTimer t("ByteView");
						imageData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(InByteView.bytes.data()), uint32_t(InByteView.bytes.size()), &width, &height, nullptr, STBI_rgb_alpha);
					},
					[&](fastgltf::sources::BufferView& InBufferView)
					{
						ScopedTimer t("BufferView");
						auto& view = asset->bufferViews[InBufferView.bufferViewIndex];
						auto& buffer = asset->buffers[view.bufferIndex];
						auto* bytes = fastgltf::DefaultBufferDataAdapter{}(buffer) + view.byteOffset;
						imageData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(bytes), uint32_t(view.byteLength), &width, &height, nullptr, STBI_rgb_alpha);
					},
					[&](auto&) {}
				}, imageInfo.data);

				YUKI_VERIFY(imageData);

				auto& image = meshData.Images[i];
				image.Width = uint32_t(width);
				image.Height = uint32_t(height);
				image.Data = reinterpret_cast<std::byte*>(imageData);
			}
		};
		imageLoadJob.AddSignal(&imageUploadBarrier);

		auto[imageUploadIndex, imageUploadJob] = m_LoadJobs.EmplaceBack();
		imageUploadJob.Task = [context = m_Context, &meshData, &mesh, stagingBuffer = m_ImageStagingBuffer](size_t InThreadID) mutable
		{
			for (size_t i = 0; i < meshData.Images.size(); i++)
			{
				LogInfo("Uploading textures");
				Fence fence = context->CreateFence();

				const auto& imageData = meshData.Images[i];
				bool shouldBlit = imageData.Width > 1024 && imageData.Height > 1024;
				Queue queue = shouldBlit ? context->GetGraphicsQueue(1) : context->GetTransferQueue();

				auto commandPool = context->CreateCommandPool(queue);
				auto imageCommandList = context->CreateCommandList(commandPool);
				context->CommandListBegin(imageCommandList);

				Image blittedImage{};
				Image image = context->CreateImage(imageData.Width, imageData.Height, ImageFormat::RGBA8UNorm, ImageUsage::Sampled | ImageUsage::TransferSource | Yuki::ImageUsage::TransferDestination);
				context->CommandListTransitionImage(imageCommandList, image, ImageLayout::ShaderReadOnly);
				context->BufferSetData(stagingBuffer, imageData.Data, imageData.Width * imageData.Height * 4);
				context->CommandListCopyToImage(imageCommandList, image, stagingBuffer, 0);

				if (shouldBlit)
				{
					blittedImage = context->CreateImage(1024, 1024, ImageFormat::RGBA8UNorm, ImageUsage::Sampled | ImageUsage::TransferDestination);
					context->CommandListTransitionImage(imageCommandList, blittedImage, ImageLayout::ShaderReadOnly);
					context->CommandListBlitImage(imageCommandList, blittedImage, image);
				}

				context->CommandListEnd(imageCommandList);
				context->QueueSubmitCommandLists(queue, { imageCommandList }, {}, { fence });
				context->FenceWait(fence);
				context->Destroy(fence);
				context->Destroy(commandPool);

				if (shouldBlit)
				{
					context->Destroy(image);
					image = blittedImage;
				}

				stbi_image_free(imageData.Data);

				mesh.Textures[i] = image;
				LogInfo("Done uploading textures!");
			}
		};
		imageUploadJob.AddSignal(&instanceCreateBarrier);
		imageUploadBarrier.Pending.push_back(&imageUploadJob);
		m_JobSystem.Schedule(&imageLoadJob);
#else
		auto[barrierIndex0, imageUploadBarrier] = m_Barriers.EmplaceBack();
		auto[imageUploadIndex, imageUploadJob] = m_LoadJobs.EmplaceBack();
		imageUploadJob.Task = [context = m_Context, &meshData, &mesh, stagingBuffer = m_ImageStagingBuffer](size_t InThreadID) mutable
		{
			LogInfo("Uploading textures");
			for (size_t i = 0; i < meshData.Images.size(); i++)
			{
				Fence fence = context->CreateFence();

				const auto& imageData = meshData.Images[i];
				bool shouldBlit = imageData.Width > 1024 && imageData.Height > 1024;
				Queue queue = shouldBlit ? context->GetGraphicsQueue(1) : context->GetTransferQueue();

				auto commandPool = context->CreateCommandPool(queue);
				auto imageCommandList = context->CreateCommandList(commandPool);
				context->CommandListBegin(imageCommandList);

				Image blittedImage{};
				Image image = context->CreateImage(imageData.Width, imageData.Height, ImageFormat::RGBA8UNorm, ImageUsage::Sampled | ImageUsage::TransferSource | Yuki::ImageUsage::TransferDestination);
				context->CommandListTransitionImage(imageCommandList, image, ImageLayout::ShaderReadOnly);
				context->BufferSetData(stagingBuffer, imageData.Data, imageData.Width * imageData.Height * 4);
				context->CommandListCopyToImage(imageCommandList, image, stagingBuffer, 0);

				if (shouldBlit)
				{
					blittedImage = context->CreateImage(1024, 1024, ImageFormat::RGBA8UNorm, ImageUsage::Sampled | ImageUsage::TransferDestination);
					context->CommandListTransitionImage(imageCommandList, blittedImage, ImageLayout::ShaderReadOnly);
					context->CommandListBlitImage(imageCommandList, blittedImage, image);
				}

				context->CommandListEnd(imageCommandList);
				context->QueueSubmitCommandLists(queue, { imageCommandList }, {}, { fence });
				context->FenceWait(fence);
				context->Destroy(fence);
				context->Destroy(commandPool);

				if (shouldBlit)
				{
					context->Destroy(image);
					image = blittedImage;
				}

				stbi_image_free(imageData.Data);

				mesh.Textures[i] = image;
			}
				LogInfo("Done uploading textures!");
		};
		imageUploadJob.AddSignal(&instanceCreateBarrier);
		imageUploadBarrier.Pending.push_back(&imageUploadJob);

		for (size_t i = 0; i < assetPtr->textures.size(); i++)
		{
			auto[index, imageLoadJob] = m_LoadJobs.EmplaceBack();
			imageLoadJob.Task = [asset = assetPtr, textureIndex = i, &mesh, &meshData, basePath = InFilePath.parent_path()](size_t InThreadID)
			{
				auto& textureInfo = asset->textures[textureIndex];

				YUKI_VERIFY(textureInfo.imageIndex.has_value());
				auto& imageInfo = asset->images[textureInfo.imageIndex.value()];

				int width, height;
				stbi_uc* imageData = nullptr;
				std::visit(ImageVisitor
				{
					[&](fastgltf::sources::URI& InURI)
					{
						ScopedTimer t("URI");
						imageData = stbi_load(fmt::format("{}/{}", basePath.string(), InURI.uri.path()).c_str(), &width, &height, nullptr, STBI_rgb_alpha);
					},
					[&](fastgltf::sources::Vector& InVector)
					{
						ScopedTimer t("Vector");
						imageData = stbi_load_from_memory(InVector.bytes.data(), uint32_t(InVector.bytes.size()), &width, &height, nullptr, STBI_rgb_alpha);
					},
					[&](fastgltf::sources::ByteView& InByteView)
					{
						ScopedTimer t("ByteView");
						imageData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(InByteView.bytes.data()), uint32_t(InByteView.bytes.size()), &width, &height, nullptr, STBI_rgb_alpha);
					},
					[&](fastgltf::sources::BufferView& InBufferView)
					{
						ScopedTimer t("BufferView");
						auto& view = asset->bufferViews[InBufferView.bufferViewIndex];
						auto& buffer = asset->buffers[view.bufferIndex];
						auto* bytes = fastgltf::DefaultBufferDataAdapter{}(buffer) + view.byteOffset;
						imageData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(bytes), uint32_t(view.byteLength), &width, &height, nullptr, STBI_rgb_alpha);
					},
					[&](auto&) {}
				}, imageInfo.data);

				YUKI_VERIFY(imageData);

				auto& image = meshData.Images[textureIndex];
				image.Width = uint32_t(width);
				image.Height = uint32_t(height);
				image.Data = reinterpret_cast<std::byte*>(imageData);
			};

			imageLoadJob.AddSignal(&imageUploadBarrier);
			m_JobSystem.Schedule(&imageLoadJob);
		}
		
#endif
		auto[materialJobIndex, materialLoadJob] = m_LoadJobs.EmplaceBack();
		materialLoadJob.Task = [asset = assetPtr, &mesh, &meshData, basePath = InFilePath.parent_path()](size_t InThreadID)
		{
			LogInfo("Reading Materials");
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
			LogInfo("Done reading materials");
		};
		m_JobSystem.Schedule(&materialLoadJob);

		auto[barrierIndex1, vertexUploadBarrier] = m_Barriers.EmplaceBack();

		auto[vertexLoadIndex, vertexLoadJob] = m_LoadJobs.EmplaceBack();
		vertexLoadJob.Task = [asset = assetPtr, &meshData, basePath = InFilePath.parent_path()](size_t InThreadID)
		{
			LogInfo("Loading Vertex Data");
			for (size_t i = 0; i < asset->meshes.size(); i++)
			{
				auto& gltfMesh = asset->meshes[i];
				MeshSourceData& sourceData = meshData.SourceData[i];

				for (auto& primitive : gltfMesh.primitives)
				{
					if (primitive.attributes.find("POSITION") == primitive.attributes.end())
						continue;

					auto& positionAccessor = asset->accessors[primitive.attributes["POSITION"]];

					if (!primitive.indicesAccessor.has_value())
						break;

					size_t baseVertexOffset = sourceData.Vertices.size();
					size_t vertexID = baseVertexOffset;
					sourceData.Vertices.resize(baseVertexOffset + positionAccessor.count);

					{
						auto& indicesAccessor = asset->accessors[primitive.indicesAccessor.value()];
						fastgltf::iterateAccessor<uint32_t>(*asset, indicesAccessor, [&](uint32_t InIndex)
						{
							sourceData.Indices.emplace_back(InIndex + uint32_t(baseVertexOffset));
						});
					}

					{
						fastgltf::iterateAccessor<Math::Vec3>(*asset, positionAccessor, [&](Math::Vec3 InPosition)
						{
							sourceData.Vertices[vertexID].Position = InPosition;
							sourceData.Vertices[vertexID].MaterialIndex = uint32_t(primitive.materialIndex.value_or(0));
							vertexID++;
						});
						vertexID = baseVertexOffset;
					}

					if (primitive.attributes.contains("NORMAL"))
					{
						auto& normalsAccessor = asset->accessors[primitive.attributes["NORMAL"]];
						fastgltf::iterateAccessor<Math::Vec3>(*asset, normalsAccessor, [&](Math::Vec3 InNormal)
						{
							sourceData.Vertices[vertexID++].Normal = InNormal;
						});
						vertexID = baseVertexOffset;
					}

					if (primitive.attributes.contains("TEXCOORD_0"))
					{
						auto& uvAccessor = asset->accessors[primitive.attributes["TEXCOORD_0"]];
						fastgltf::iterateAccessor<Math::Vec2>(*asset, uvAccessor, [&](Math::Vec2 InUV)
						{
							sourceData.Vertices[vertexID++].UV = InUV;
						});
						vertexID = baseVertexOffset;
					}
				}
			}

			LogInfo("Done loading vertex data");
		};
		vertexLoadJob.AddSignal(&vertexUploadBarrier);

		auto[vertexUploadIndex, vertexUploadJob] = m_LoadJobs.EmplaceBack();
		vertexUploadJob.Task = [context = m_Context, &meshData, &mesh, stagingBuffer = m_MeshStagingBuffer](size_t InThreadID) mutable
		{
			LogInfo("Uploading vertex data");

			auto commandPool = context->CreateCommandPool(context->GetTransferQueue(1));
			Fence fence = context->CreateFence();

			for (size_t i = 0; i < meshData.SourceData.size(); i++)
			{
				auto& meshSource = mesh.Sources[i];
				const auto& sourceData = meshData.SourceData[i];

				{
					uint32_t vertexDataSize = uint32_t(sourceData.Vertices.size()) * sizeof(Vertex);

					meshSource.VertexData = context->CreateBuffer({
						.Type = BufferType::StorageBuffer,
						.Size = vertexDataSize
					});

					if (meshSource.VertexData != Buffer{})
					{
						for (uint32_t bufferOffset = 0; bufferOffset < vertexDataSize; bufferOffset += s_StagingBufferSize)
						{
							auto commandList = context->CreateCommandList(commandPool);
							context->CommandListBegin(commandList);

							const std::byte* ptr = (const std::byte*)sourceData.Vertices.data();
							uint32_t blockSize = std::min(bufferOffset + s_StagingBufferSize, vertexDataSize) - bufferOffset;
							context->BufferSetData(stagingBuffer, ptr + bufferOffset, blockSize, 0);
							context->CommandListCopyToBuffer(commandList, meshSource.VertexData, bufferOffset, stagingBuffer, 0, blockSize);

							context->CommandListEnd(commandList);
							context->QueueSubmitCommandLists(context->GetTransferQueue(1), { commandList }, {}, { fence });
							context->FenceWait(fence);
						}
					}
				}

				{
					uint32_t indexDataSize = uint32_t(sourceData.Indices.size()) * sizeof(uint32_t);

					meshSource.IndexBuffer = context->CreateBuffer({
						.Type = BufferType::IndexBuffer,
						.Size = indexDataSize
					});

					if (meshSource.IndexBuffer != Buffer{})
					{
						for (uint32_t bufferOffset = 0; bufferOffset < indexDataSize; bufferOffset += s_StagingBufferSize)
						{
							auto commandList = context->CreateCommandList(commandPool);
							context->CommandListBegin(commandList);

							const std::byte* ptr = (const std::byte*)sourceData.Indices.data();
							uint32_t blockSize = std::min(bufferOffset + s_StagingBufferSize, indexDataSize) - bufferOffset;
							context->BufferSetData(stagingBuffer, ptr + bufferOffset, blockSize, 0);
							context->CommandListCopyToBuffer(commandList, meshSource.IndexBuffer, bufferOffset, stagingBuffer, 0, blockSize);

							context->CommandListEnd(commandList);
							context->QueueSubmitCommandLists(context->GetTransferQueue(1), { commandList }, {}, { fence });
							context->FenceWait(fence);
						}

						meshSource.IndexCount = uint32_t(sourceData.Indices.size());
					}
				}
			}

			context->Destroy(fence);
			context->Destroy(commandPool);

			LogInfo("Done uploading vertex data");
		};
		vertexUploadJob.AddSignal(&instanceCreateBarrier);
		vertexUploadBarrier.Pending.push_back(&vertexUploadJob);
		m_JobSystem.Schedule(&vertexLoadJob);

		auto[instanceCreateJobIndex, instanceCreateJob] = m_LoadJobs.EmplaceBack();
		instanceCreateJob.Task = [asset = assetPtr, scene, &mesh, &meshData, this](size_t InThreadID) mutable
		{
			Math::Mat4 transform;
			transform.SetIdentity();

			for (auto nodeIndex : scene->nodeIndices)
				ProcessNodeHierarchy(asset, mesh, nodeIndex, transform);

			mesh.Instances.shrink_to_fit();
			m_Callback(std::move(mesh));
		};
		instanceCreateBarrier.Pending.push_back(&instanceCreateJob);
	}
}

