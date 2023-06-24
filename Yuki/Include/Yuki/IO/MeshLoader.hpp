#pragma once

#include "Yuki/Core/JobSystem.hpp"
#include "Yuki/Rendering/MeshData.hpp"
#include "Yuki/Rendering/RenderContext.hpp"

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
		struct MeshData
		{
			DynamicArray<Vertex> Vertices;
			DynamicArray<uint32_t> Indices;
		};

		DynamicArray<std::pair<Buffer, Buffer>> CreateGPUBuffers(const DynamicArray<MeshData>& InMeshes);

	private:
		RenderContext* m_Context = nullptr;
		CommandPool m_CommandPool{};
		Buffer m_StagingBuffer{};

		PushMeshCallback m_Callback;

		std::shared_mutex m_UploadQueueMutex;
		DynamicArray<std::pair<Mesh, DynamicArray<MeshData>>> m_UploadQueue;

		JobSystem m_JobSystem;

	};

}
