#pragma once

#include "MeshData.hpp"
#include "RHI/RenderContext.hpp"
#include "RHI/RenderInterface.hpp"
#include "RHI/CommandBufferPool.hpp"
#include "RHI/GraphicsPipeline.hpp"
#include "RHI/DescriptorSet.hpp"
#include "RHI/Sampler.hpp"
	
namespace Yuki {

	class SceneRenderer
	{
	public:
		SceneRenderer(RenderContext* InContext, Viewport* InViewport);

		void BeginDraw(const Math::Mat4& InViewMatrix);
		void DrawMesh(LoadedMesh& InMesh);
		void EndDraw();

		CommandBuffer* GetCurrentCommandBuffer() const { return m_CommandBuffer; }

	private:
		void CreateDescriptorSets();
		void BuildPipelines();

	private:
		RenderContext* m_Context = nullptr;
		RenderInterface* m_RenderInterface = nullptr;
		CommandBufferPool* m_CommandPool = nullptr;
		CommandBuffer* m_CommandBuffer = nullptr;

		Buffer* m_StagingBuffer = nullptr;

		Viewport* m_Viewport = nullptr;

		DescriptorPool* m_DescriptorPool = nullptr;
		DescriptorSet* m_MaterialDescriptorSet = nullptr;

		Sampler* m_Sampler = nullptr;

		Buffer* m_MaterialStorageBuffer = nullptr;

		Unique<GraphicsPipeline> m_MeshPipeline = nullptr;

		struct FrameTransforms
		{
			Math::Mat4 ViewProjection;
			Math::Mat4 Transform;
		} m_FrameTransforms;
	};

}
