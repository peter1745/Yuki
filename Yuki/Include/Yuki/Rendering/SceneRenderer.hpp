#pragma once

#include "MeshData.hpp"
#include "RenderContext.hpp"

#include "Yuki/Math/Mat4.hpp"

namespace Yuki {

	class SceneRenderer
	{
	public:
		SceneRenderer(RenderContext* InContext, Swapchain InSwapchain);

		void BeginFrame(const Math::Mat4& InViewProjection);
		void EndFrame(Fence InFence);

		void Submit(LoadedMesh& InMesh);

		void SetWireframeMode(bool InEnable) { m_ActivePipeline = InEnable ? m_WireframePipeline : m_Pipeline; }

	private:
		void CreateDescriptorSets();
		void CreatePipelines();

	private:
		RenderContext* m_Context = nullptr;
		Swapchain m_TargetSwapchain{};

		Shader m_MeshShader{};
		Pipeline m_Pipeline{};
		Pipeline m_WireframePipeline{};
		Pipeline m_ActivePipeline{};

		CommandPool m_CommandPool{};
		CommandList m_CommandList{};

		Sampler m_Sampler{};

		Buffer m_StagingBuffer{};
		Buffer m_MaterialsBuffer{};

		DescriptorPool m_DescriptorPool{};
		DescriptorSetLayout m_DescriptorSetLayout{};
		DescriptorSet m_MaterialSet{};

		struct FrameTransforms
		{
			Math::Mat4 ViewProjection;
			Math::Mat4 Transform;
		} m_FrameTransforms;
	};

}
