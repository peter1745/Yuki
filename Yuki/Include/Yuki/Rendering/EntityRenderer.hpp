#pragma once

#include "Yuki/Core/ResourceRegistry.hpp"
#include "Yuki/Math/Mat4.hpp"
#include "Yuki/World/World.hpp"
#include "Yuki/Entities/RenderingComponents.hpp"

#include "RenderResources.hpp"

namespace Yuki {

	class RenderContext;

	class WorldRenderer
	{
	public:
		WorldRenderer(RenderContext* InContext, World& InWorld);

		void CreateGPUObject(flecs::entity InEntity);
		//MeshHandle SubmitForUpload(Mesh InMesh);

		void Reset();
		void PrepareFrame();
		void BeginFrame(const Math::Mat4& InViewProjection);
		void RenderEntities();
		void EndFrame();

		void SetViewportSize(uint32_t InWidth, uint32_t InHeight);

		void SynchronizeGPUTransform(flecs::entity InEntity);

		Image GetFinalImage() { return m_ColorImage; }
		
		//Mesh& GetMesh(MeshHandle InHandle) { return m_Meshes.Get(InHandle); }
		//const Mesh& GetMesh(MeshHandle InHandle) const { return m_Meshes.Get(InHandle); }

	public:
		flecs::entity PreRenderPhase;
		flecs::entity RenderPhase;
		flecs::entity PostRenderPhase;

	private:
		RenderContext* m_Context = nullptr;
		World& m_World;
		flecs::system m_PreRenderSystem{};
		flecs::system m_RenderSystem{};
		flecs::system m_PostRenderSystem{};

		Image m_ColorImage{};
		Image m_DepthImage{};

		Queue m_GraphicsQueue{};

		Shader m_Shader{};
		Pipeline m_Pipeline{};
		Pipeline m_WireframePipeline{};

		DescriptorPool m_DescriptorPool{};
		DescriptorSetLayout m_DescriptorSetLayout{};
		DescriptorSet m_MaterialSet{};

		Sampler m_Sampler{};

		CommandPool m_CommandPool{};
		CommandList m_CommandList{};

		Fence m_Fence{};

		//ResourceRegistry<MeshHandle, Mesh> m_Meshes;
		//std::shared_mutex m_MeshUploadMutex;
		//DynamicArray<MeshHandle> m_MeshUploadQueue;

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
			uint32_t BaseTextureOffset;
		};

		struct CPUObject
		{
			//MeshHandle Mesh;
			size_t InstanceIndex;
		};

		DynamicArray<CPUObject> m_CPUObjects;

		Buffer m_ObjectStorageBuffer{};
		Buffer m_ObjectStagingBuffer{};
		Buffer m_TransformStorageBuffer{};
		Buffer m_TransformStagingBuffer{};

		uint32_t m_TextureCount = 0;

		uint32_t m_LastInstanceID = 0;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		Map<flecs::entity, uint32_t, FlecsEntityHash> m_EntityInstanceMap;
	};

}
