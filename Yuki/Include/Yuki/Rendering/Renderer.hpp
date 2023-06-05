#pragma once

#include "Yuki/Rendering/RHI/RenderInterface.hpp"
#include "Yuki/Rendering/RHI/GraphicsPipelineBuilder.hpp"
#include "Yuki/Rendering/RHI/Buffer.hpp"

namespace Yuki {

	class Renderer
	{
	public:
		Renderer(RenderContext* InContext);
		~Renderer();

		void Begin();
		void DrawTriangle();
		void End();

		CommandBuffer* GetCommandBuffer() const { return m_CommandBuffer; }

	private:
		void BuildPipelines();

	private:
		RenderContext* m_Context = nullptr;
		RenderInterface* m_RenderInterface = nullptr;
		Unique<GraphicsPipeline> m_TrianglePipeline = nullptr;
		CommandBuffer* m_CommandBuffer = nullptr;

		Buffer* m_StagingBuffer = nullptr;
	};

}
