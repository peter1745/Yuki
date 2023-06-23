#pragma once

#include "MeshData.hpp"
//#include "RHI/RenderContext.hpp"
//#include "RHI/RenderInterface.hpp"
//#include "RHI/CommandBufferPool.hpp"
//#include "RHI/GraphicsPipeline.hpp"
//#include "RHI/DescriptorSet.hpp"
//#include "RHI/Sampler.hpp"
	
namespace Yuki {

	class SceneRenderer
	{
	public:
		//SceneRenderer(RenderContext* InContext);

		//void SetTargetViewport(Viewport* InViewport);
		void BeginFrame();
		void BeginDraw(const Math::Mat4& InViewMatrix);
		//void DrawMesh(LoadedMesh& InMesh);
		void EndDraw();
		void EndFrame();

		//CommandBuffer* GetCurrentCommandBuffer() const { return m_CommandBuffer; }

	private:
		void CreateDescriptorSets();
		void BuildPipelines();

	private:
		//RenderContext* m_Context = nullptr;
		//Unique<RenderInterface> m_RenderInterface = nullptr;
		//Unique<CommandBufferPool> m_CommandPool = nullptr;
		//CommandBuffer* m_CommandBuffer = nullptr;
//
		//Unique<Buffer> m_StagingBuffer = nullptr;
//
		//Viewport* m_Viewport = nullptr;
//
		//Unique<DescriptorPool> m_DescriptorPool = nullptr;
		//DescriptorSet* m_MaterialDescriptorSet = nullptr;
//
		//Unique<Sampler> m_Sampler = nullptr;
//
		//Unique<Buffer> m_MaterialStorageBuffer = nullptr;
//
		//Unique<Shader> m_MeshShader = nullptr;
		//Unique<GraphicsPipeline> m_MeshPipeline = nullptr;

		struct FrameTransforms
		{
			Math::Mat4 ViewProjection;
			Math::Mat4 Transform;
		} m_FrameTransforms;
	};

}
