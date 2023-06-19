#pragma once

namespace Yuki {

	class Buffer;
	struct GraphicsPipeline;
	class Viewport;
	struct RenderTarget;

	class CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void BindVertexBuffer(Buffer* InVertexBuffer) = 0;
		virtual void BindPipeline(GraphicsPipeline* InPipeline) = 0;

		virtual void SetViewport(Viewport* InViewport) = 0;

		virtual void BeginRendering(RenderTarget* InRenderTarget) = 0;
		virtual void BeginRendering(Viewport* InViewport) = 0;
		virtual void EndRendering() = 0;

		virtual void Draw(uint32_t InVertexCount, uint32_t InInstanceCount, uint32_t InFirstVertex, uint32_t InFirstInstance) = 0;

		template<typename T>
		T As() const { return static_cast<T>(GetUnderlyingHandle()); }

	private:
		virtual void* GetUnderlyingHandle() const = 0;
	};

}
