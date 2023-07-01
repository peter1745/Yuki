#pragma once

#include "Yuki/Core/JobSystem.hpp"
#include "Yuki/Core/StableDynamicArray.hpp"
#include "Yuki/Rendering/MeshData.hpp"
#include "Yuki/Rendering/RenderResources.hpp"

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
		struct MeshSourceData
		{
			DynamicArray<Vertex> Vertices;
			DynamicArray<uint32_t> Indices;
		};

		struct MeshData
		{
			DynamicArray<MeshSourceData> SourceData;

			struct ImageData
			{
				uint32_t Width;
				uint32_t Height;
				std::byte *Data = nullptr;
			};
			DynamicArray<ImageData> Images;
		};

	private:
		RenderContext* m_Context = nullptr;
		Buffer m_MeshStagingBuffer{};
		Buffer m_ImageStagingBuffer{};
		Buffer m_MaterialStagingBuffer{};

		PushMeshCallback m_Callback;

		StableDynamicArray<MeshData, 100> m_ProcessingQueue;
		StableDynamicArray<Mesh, 100> m_MeshQueue;

		JobSystem m_JobSystem;

		StableDynamicArray<Job, 100> m_LoadJobs;
		StableDynamicArray<Barrier, 100> m_Barriers;

	};

}
