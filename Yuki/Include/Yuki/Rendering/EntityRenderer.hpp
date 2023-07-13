#pragma once

#include "Yuki/Core/ResourceRegistry.hpp"
#include "Yuki/Math/Mat4.hpp"
#include "Yuki/World/World.hpp"
#include "Yuki/Entities/RenderingComponents.hpp"
#include "Yuki/Asset/AssetSystem.hpp"

#include "Renderer.hpp"

namespace Yuki {

	class WorldRenderer : public Renderer
	{
	public:
		WorldRenderer(World& InWorld, RenderContext* InContext);

		void CreateGPUInstance(flecs::entity InRoot);
		void SubmitForUpload(AssetID InAssetID, AssetSystem& InAssetSystem, const MeshScene& InMeshScene);

		void Reset();
		void PrepareFrame();
		void BeginFrame(const Math::Mat4& InViewProjection);
		void RenderEntities();
		void EndFrame();

		void SetViewportSize(uint32_t InWidth, uint32_t InHeight);

		void SynchronizeGPUTransform(flecs::entity InEntity);

		Image GetFinalImage() { return m_ColorImage; }

	private:
		World& m_World;

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

		struct GPUMesh
		{
			Buffer VertexData;
			Buffer IndexData;
			uint32_t IndexCount;
			Math::Mat4 Transform;
			bool IsReady = false;
		};

		struct GPUMeshScene
		{
			DynamicArray<GPUMesh> Meshes;
			Buffer MaterialData;
			uint32_t BaseTextureOffset;
			DynamicArray<Image> Textures;
			DescriptorSet TextureSet;
			Barrier UploadBarrier;
		};
		Map<AssetID, GPUMeshScene, AssetIDHash> m_GPUMeshScenes;

		StableDynamicArray<Job> m_UploadFinishedJobs;

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
		std::atomic<uint32_t> m_GPUObjectCount = 0;

		std::shared_mutex m_RenderMutex;

		Buffer m_ObjectStorageBuffer{};
		Buffer m_ObjectStagingBuffer{};
		Buffer m_TransformStorageBuffer{};

		uint32_t m_TextureCount = 0;

		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		Map<flecs::entity, uint32_t, FlecsEntityHash> m_EntityInstanceMap;
	};

}
