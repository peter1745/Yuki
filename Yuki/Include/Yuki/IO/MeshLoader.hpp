#pragma once

#include "Yuki/Core/JobSystem.hpp"
#include "Yuki/Rendering/MeshData.hpp"
#include "Yuki/Rendering/RenderContext.hpp"

namespace fastgltf {
	struct Asset;
}

namespace Yuki {

	class RenderContext;

	template<typename... TLambdas>
	struct ImageVisitor : TLambdas... { using TLambdas::operator()...; };
	template<typename... TLambdas>
	ImageVisitor(TLambdas...) -> ImageVisitor<TLambdas...>;

	class MeshLoader
	{
	public:
		using PushMeshCallback = std::function<void(Mesh)>;

		MeshLoader(RenderContext* InContext, PushMeshCallback InCallback);

		void LoadGLTFMesh(const std::filesystem::path& InFilePath);

	private:
		struct MeshInstanceData
		{
			DynamicArray<Vertex> Vertices;
			DynamicArray<uint32_t> Indices;
		};

		struct MeshData
		{
			DynamicArray<MeshInstanceData> InstanceData;

			struct ImageData
			{
				uint32_t Width;
				uint32_t Height;
				DynamicArray<std::byte> Data;
			};
			DynamicArray<ImageData> Images;
		};

		void ProcessMaterials(fastgltf::Asset* InAsset, const std::filesystem::path& InBasePath, MeshData& InMeshData);

	private:
		RenderContext* m_Context = nullptr;
		CommandPool m_CommandPool{};
		Buffer m_StagingBuffer{};

		PushMeshCallback m_Callback;

		std::shared_mutex m_UploadQueueMutex;
		DynamicArray<std::pair<Mesh, MeshData>> m_UploadQueue;

		JobSystem m_JobSystem;

	};

}
