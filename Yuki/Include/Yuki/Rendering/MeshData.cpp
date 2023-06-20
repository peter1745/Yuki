#include "MeshData.hpp"

namespace Yuki {

	static Buffer* s_StagingBuffer = nullptr;

	Mesh Mesh::FromMeshData(RenderContext* InContext, const MeshData& InMeshData)
	{
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

		Mesh mesh = {};

		{
			uint32_t vertexDataSize = sizeof(Vertex) * uint32_t(InMeshData.Vertices.size());

			s_StagingBuffer->SetData(InMeshData.Vertices.data(), vertexDataSize);

			Yuki::BufferInfo bufferInfo =
			{
				.Type = BufferType::VertexBuffer,
				.Size = vertexDataSize
			};
			mesh.VertexBuffer = InContext->CreateBuffer(bufferInfo);
			YUKI_VERIFY(bufferInfo.Size < 100 * 1024 * 1024);
			mesh.VertexBuffer->UploadData(s_StagingBuffer);
		}

		{
			uint32_t indexDataSize = sizeof(uint32_t) * uint32_t(InMeshData.Indices.size());

			s_StagingBuffer->SetData(InMeshData.Indices.data(), indexDataSize);

			Yuki::BufferInfo bufferInfo =
			{
				.Type = BufferType::IndexBuffer,
				.Size = indexDataSize
			};
			mesh.IndexBuffer = InContext->CreateBuffer(bufferInfo);
			YUKI_VERIFY(bufferInfo.Size < 100 * 1024 * 1024);
			mesh.IndexBuffer->UploadData(s_StagingBuffer);

			mesh.IndexCount = InMeshData.Indices.size();
		}

		return mesh;
	}

}
