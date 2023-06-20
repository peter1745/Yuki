#pragma once

#include "MeshData.hpp"
#include "RHI/RenderContext.hpp"
#include "RHI/RenderInterface.hpp"
#include "RHI/GraphicsPipelineBuilder.hpp"
#include "RHI/CommandBufferPool.hpp"
	
namespace Yuki {

	class SceneRenderer
	{
	public:
		SceneRenderer(RenderContext* InContext, Viewport* InViewport);

		void BeginDraw(const Math::Mat4& InViewMatrix);
		void DrawMesh(const LoadedMesh& InMesh);
		void EndDraw();

		CommandBuffer* GetCurrentCommandBuffer() const { return m_CommandBuffer; }

	private:
		void BuildPipelines();

	private:
		RenderContext* m_Context = nullptr;
		RenderInterface* m_RenderInterface = nullptr;
		CommandBufferPool* m_CommandPool = nullptr;
		CommandBuffer* m_CommandBuffer = nullptr;

		Viewport* m_Viewport = nullptr;

		Unique<GraphicsPipeline> m_MeshPipeline = nullptr;

		struct FrameTransforms
		{
			Math::Mat4 ViewProjection;
			Math::Mat4 Transform;
		} m_FrameTransforms;

	};

}
