#include "IO/MeshLoader.hpp"
#include "Core/Timer.hpp"

#if 0
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
#endif

namespace Yuki {

	static constexpr uint32_t s_StagingBufferSize = 100 * 1024 * 1024;

#if 0
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
			if (sourceMesh.VertexData.Handle != BufferHandle{} && sourceMesh.IndexBuffer.Handle != BufferHandle{} && sourceMesh.IndexCount > 0)
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
		InCommandList.End();
		InContext->QueueSubmitCommandLists(InContext->GetTransferQueue(), { InCommandList }, {}, {});
		InContext->QueueWaitIdle(InContext->GetTransferQueue());

		InContext->CommandPoolReset(InCommandPool);
		InContext->CommandListBegin(InCommandList);
	}
#endif

	/*MeshLoader::MeshLoader(RenderContext* InContext, PushMeshCallback InCallback)
		: m_Context(InContext), m_Callback(std::move(InCallback))
	{
		m_JobSystem.Init(4);

		m_MeshStagingBuffer = Buffer(m_Context, {
			.Type = BufferType::StagingBuffer,
			.Size = s_StagingBufferSize
		});

		m_ImageStagingBuffer = Buffer(m_Context, {
			.Type = BufferType::StagingBuffer,
			.Size = s_StagingBufferSize
		});

		m_MaterialStagingBuffer = Buffer(m_Context, {
			.Type = BufferType::StagingBuffer,
			.Size = 100 * 1024 * 1024
		});
	}*/

	void MeshLoader::LoadGLTFMesh(const std::filesystem::path& InFilePath)
	{
	#if 0
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
		mesh.FilePath = InFilePath;
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

		if (!assetPtr->textures.empty())
		{
			auto[barrierIndex0, imageUploadBarrier] = m_Barriers.EmplaceBack();
			auto[imageUploadIndex, imageUploadJob] = m_LoadJobs.EmplaceBack();
			imageUploadJob.Task = [context = m_Context, &meshData, &mesh, stagingBuffer = m_ImageStagingBuffer](size_t InThreadID) mutable
			{
				LogInfo("Uploading textures");
				for (size_t i = 0; i < meshData.Images.size(); i++)
				{
					Fence fence{context};

					const auto& imageData = meshData.Images[i];
					bool shouldBlit = imageData.Width > 1024 && imageData.Height > 1024;
					
					Queue queue = { shouldBlit ? context->GetGraphicsQueue(1) : context->GetTransferQueue(), context };

					auto commandPool = CommandPool(context, queue);
					auto imageCommandList = commandPool.CreateCommandList();
					imageCommandList.Begin();

					Image blittedImage{};
					Image image{context, imageData.Width, imageData.Height, ImageFormat::RGBA8UNorm, ImageUsage::Sampled | ImageUsage::TransferSource | Yuki::ImageUsage::TransferDestination};
					imageCommandList.TransitionImage(image, ImageLayout::ShaderReadOnly);
					stagingBuffer.SetData(imageData.Data, imageData.Width * imageData.Height * 4);
					imageCommandList.CopyToImage(image, stagingBuffer, 0);

					if (shouldBlit)
					{
						blittedImage = Image(context, 1024, 1024, ImageFormat::RGBA8UNorm, ImageUsage::Sampled | ImageUsage::TransferDestination);
						imageCommandList.TransitionImage(blittedImage, ImageLayout::ShaderReadOnly);
						imageCommandList.BlitImage(blittedImage, image);
					}

					imageCommandList.End();
					queue.SubmitCommandLists({ imageCommandList }, {}, { fence });
					
					fence.Wait();

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
					LogInfo("Reading Texture");
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
					LogInfo("Done reading texture");
				};

				imageLoadJob.AddSignal(&imageUploadBarrier);
				m_JobSystem.Schedule(&imageLoadJob);
			}
		}
		
		auto[materialJobIndex, materialLoadJob] = m_LoadJobs.EmplaceBack();
		auto[materialBarrierIndex, materialBarrier] = m_Barriers.EmplaceBack();
		materialLoadJob.Task = [asset = assetPtr, &mesh, &meshData, basePath = InFilePath.parent_path()](size_t InThreadID)
		{
			LogInfo("Reading Materials");
			for (auto& material : asset->materials)
			{
				auto& meshMaterial = mesh.Materials.emplace_back();

				if (!material.pbrData.has_value())
					continue;

				auto& pbrData = material.pbrData.value();

				meshMaterial.AlbedoColor = {
					pbrData.baseColorFactor[0],
					pbrData.baseColorFactor[1],
					pbrData.baseColorFactor[2],
					pbrData.baseColorFactor[3]
				};
				
				if (!pbrData.baseColorTexture.has_value())
					continue;

				auto& colorTexture = pbrData.baseColorTexture.value();
				meshMaterial.AlbedoTextureIndex = int32_t(colorTexture.textureIndex);
				LogInfo("Texture Index: {}", meshMaterial.AlbedoTextureIndex);
			}
			LogInfo("Done reading materials");
		};
		materialLoadJob.AddSignal(&materialBarrier);

		auto[materialUploadJobIndex, materialUploadJob] = m_LoadJobs.EmplaceBack();
		materialUploadJob.Task = [context = m_Context, stagingBuffer = m_MaterialStagingBuffer, &mesh, &meshData, basePath = InFilePath.parent_path()](size_t InThreadID) mutable
		{
			LogInfo("Uploading Materials");
			uint32_t materialSize = uint32_t(mesh.Materials.size()) * sizeof(MeshMaterial);
			mesh.MaterialStorageBuffer = Buffer(context, {
				.Type = BufferType::StorageBuffer,
				.Size = materialSize
			});

			stagingBuffer.SetData(mesh.Materials.data(), materialSize);

			Queue graphicsQueue{ context->GetGraphicsQueue(3), context };
			CommandPool commandPool{context,  graphicsQueue};

			auto commandList = commandPool.CreateCommandList();
			commandList.Begin();
			commandList.CopyToBuffer(mesh.MaterialStorageBuffer, 0, stagingBuffer, 0, materialSize);
			commandList.End();
			graphicsQueue.SubmitCommandLists({ commandList }, {}, {});
			graphicsQueue.WaitIdle();

			context->Destroy(commandPool);

			LogInfo("Done uploading materials");
		};
		materialBarrier.Pending.push_back(&materialUploadJob);
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

			Queue transferQueue{ context->GetTransferQueue(1), context };
			CommandPool commandPool{context,  transferQueue};
			Fence fence{context};

			for (size_t i = 0; i < meshData.SourceData.size(); i++)
			{
				auto& meshSource = mesh.Sources[i];
				const auto& sourceData = meshData.SourceData[i];

				{
					uint32_t vertexDataSize = uint32_t(sourceData.Vertices.size()) * sizeof(Vertex);

					meshSource.VertexData = Buffer(context, {
						.Type = BufferType::StorageBuffer,
						.Size = vertexDataSize
					});

					if (meshSource.VertexData != BufferHandle{})
					{
						for (uint32_t bufferOffset = 0; bufferOffset < vertexDataSize; bufferOffset += s_StagingBufferSize)
						{
							auto commandList = commandPool.CreateCommandList();
							commandList.Begin();

							const std::byte* ptr = (const std::byte*)sourceData.Vertices.data();
							uint32_t blockSize = std::min(bufferOffset + s_StagingBufferSize, vertexDataSize) - bufferOffset;
							stagingBuffer.SetData(ptr + bufferOffset, blockSize, 0);
							commandList.CopyToBuffer(meshSource.VertexData, bufferOffset, stagingBuffer, 0, blockSize);

							commandList.End();
							transferQueue.SubmitCommandLists({ commandList }, {}, { fence });

							fence.Wait();
						}
					}
				}

				{
					uint32_t indexDataSize = uint32_t(sourceData.Indices.size()) * sizeof(uint32_t);

					meshSource.IndexBuffer = Buffer(context, {
						.Type = BufferType::IndexBuffer,
						.Size = indexDataSize
					});

					if (meshSource.IndexBuffer.Handle != BufferHandle{})
					{
						for (uint32_t bufferOffset = 0; bufferOffset < indexDataSize; bufferOffset += s_StagingBufferSize)
						{
							auto commandList = commandPool.CreateCommandList();
							commandList.Begin();

							const std::byte* ptr = (const std::byte*)sourceData.Indices.data();
							uint32_t blockSize = std::min(bufferOffset + s_StagingBufferSize, indexDataSize) - bufferOffset;
							stagingBuffer.SetData(ptr + bufferOffset, blockSize, 0);
							commandList.CopyToBuffer(meshSource.IndexBuffer, bufferOffset, stagingBuffer, 0, blockSize);

							commandList.End();
							transferQueue.SubmitCommandLists({ commandList }, {}, { fence });

							fence.Wait();
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
			LogInfo("Creating Instances, {} root indices", scene->nodeIndices.size());
			Math::Mat4 transform;
			transform.SetIdentity();

			for (auto nodeIndex : scene->nodeIndices)
				ProcessNodeHierarchy(asset, mesh, nodeIndex, transform);

			mesh.Instances.shrink_to_fit();
			m_Callback(std::move(mesh));
			LogInfo("Done creating instances");
		};
		instanceCreateBarrier.Pending.push_back(&instanceCreateJob);

		LogInfo("{}", instanceCreateBarrier.Counter.load());
	#endif
	}
}
