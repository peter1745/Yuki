#pragma once

#include "Yuki/Core/ResourceRegistry.hpp"
#include "Yuki/Math/Mat4.hpp"
#include "Yuki/Entities/RenderingComponents.hpp"

#include "MeshData.hpp"

#include <flecs/flecs.h>

namespace Yuki {

	class RenderContext;

	class EntityRenderer
	{
	public:
		EntityRenderer(RenderContext* InContext, SwapchainHandle InSwapchain, flecs::world& InWorld);

		MeshHandle SubmitForUpload(Mesh InMesh);

		void Reset();
		void PrepareFrame();
		void BeginFrame(const Math::Mat4& InViewProjection);
		void RenderEntities();
		void EndFrame();

	public:
		flecs::entity PreRenderPhase;
		flecs::entity RenderPhase;
		flecs::entity PostRenderPhase;

	private:
		RenderContext* m_Context = nullptr;
		flecs::world& m_World;
		flecs::system m_PreRenderSystem{};
		flecs::system m_RenderSystem{};
		flecs::system m_PostRenderSystem{};

		SwapchainHandle m_Swapchain;

		Queue m_GraphicsQueue{};

		Shader m_Shader{};
		Pipeline m_Pipeline{};

		DescriptorPool m_DescriptorPool{};
		DescriptorSetLayout m_DescriptorSetLayout{};
		DescriptorSet m_MaterialSet{};

		Sampler m_Sampler{};

		CommandPool m_CommandPool{};
		CommandList m_CommandList{};

		Fence m_Fence{};

		ResourceRegistry<MeshHandle, Mesh> m_Meshes;
		std::shared_mutex m_MeshUploadMutex;
		DynamicArray<MeshHandle> m_MeshUploadQueue;

		struct PushConstants
		{
			Math::Mat4 ViewProjection;
			uint64_t ObjectVA;
			uint64_t TransformVA;
		} m_PushConstants;

		struct GPUObject
		{
			uint64_t VertexVA;
			uint64_t MaterialVA;
		};

		Buffer m_ObjectStorageBuffer{};
		Buffer m_ObjectStagingBuffer{};
		Buffer m_TransformStorageBuffer{};
		Buffer m_TransformStagingBuffer{};

		uint32_t m_TextureCount = 0;

		uint32_t m_LastInstanceID = 0;
	};

}
