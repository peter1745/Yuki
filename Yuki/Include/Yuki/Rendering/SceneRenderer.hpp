#pragma once

#include "MeshData.hpp"
#include "RenderResources.hpp"

#include "Yuki/Math/Mat4.hpp"

namespace Yuki {

	class SceneRenderer
	{
	public:
		SceneRenderer(RenderContext* InContext, SwapchainHandle InSwapchain);

		void BeginFrame(const Math::Mat4& InViewProjection);
		void EndFrame(FenceHandle InFence);

		void Submit(Mesh& InMesh);

		void SetWireframeMode(bool InEnable) { m_ActivePipeline = InEnable ? m_WireframePipeline : m_Pipeline; }

		void RegisterMeshData(Mesh& InMesh);

		CommandList GetCommandList() { return m_CommandList; }

	private:
		void CreateDescriptorSets();
		void CreatePipelines();

	private:
		RenderContext* m_Context = nullptr;
		Swapchain m_TargetSwapchain{};
		Queue m_GraphicsQueue{};

		Shader m_MeshShader{};
		Pipeline m_Pipeline{};
		Pipeline m_WireframePipeline{};
		Pipeline m_ActivePipeline{};

		CommandPool m_CommandPool{};
		CommandList m_CommandList{};

		Sampler m_Sampler{};

		Buffer m_StagingBuffer{};
		Buffer m_MaterialsBuffer{};

		uint32_t m_MaterialCount = 0;
		uint32_t m_TextureCount = 0;

		DescriptorPool m_DescriptorPool{};
		DescriptorSetLayout m_DescriptorSetLayout{};
		DescriptorSet m_MaterialSet{};
		uint32_t m_MaterialSetOffset = 0;
		uint32_t m_MaterialSetTextureOffset = 0;

		struct PushConstants
		{
			Math::Mat4 ViewProjection;
			Math::Mat4 Transform;
			uint64_t VertexVA;
			uint64_t MaterialVA;
			uint32_t MaterialOffset;
		} m_PushConstants;
	};

}
