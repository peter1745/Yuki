#pragma once

#include "Yuki/Rendering/RHI/Image2D.hpp"
#include "Yuki/Rendering/RHI/DescriptorSet.hpp"

#include <span>

namespace Yuki {

	class Buffer;
	class GraphicsPipeline;
	class Viewport;
	struct RenderTarget;

	class CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void BindVertexBuffer(Buffer* InVertexBuffer) = 0;
		virtual void BindIndexBuffer(Buffer* InIndexBuffer) = 0;
		virtual void BindPipeline(GraphicsPipeline* InPipeline) = 0;
		virtual void BindDescriptorSets(GraphicsPipeline* InPipeline, std::span<DescriptorSet* const> InDescriptorSets) = 0;

		virtual void SetViewport(Viewport* InViewport) = 0;

		virtual void BeginRendering(RenderTarget* InRenderTarget) = 0;
		virtual void BeginRendering(Viewport* InViewport) = 0;
		virtual void EndRendering() = 0;

		virtual void PushConstants(GraphicsPipeline* InPipeline, const void* InData, uint32_t InDataSize, uint32_t InOffset) = 0;

		virtual void Draw(uint32_t InVertexCount, uint32_t InInstanceCount, uint32_t InFirstVertex, uint32_t InFirstInstance) = 0;
		virtual void DrawIndexed(uint32_t InIndexCount, uint32_t InInstanceCount, uint32_t InFirstIndex, int32_t InVertexOffset, uint32_t InFirstInstance) = 0;

		virtual void TransitionImage(Image2D* InImage, ImageLayout InNewLayout) = 0;

		virtual void CopyToBuffer(Buffer* InDstBuffer, uint32_t InDstOffset, Buffer* InSrcBuffer, uint32_t InSrcOffset, uint32_t InSize) = 0;
		virtual void CopyToImage(Image2D* InDstImage, Buffer* InSrcBuffer, uint32_t InSrcOffset) = 0;

		virtual void BlitImage(Image2D* InDstImage, Image2D* InSrcImage) = 0;

		template<typename T>
		T As() const { return static_cast<T>(GetUnderlyingHandle()); }

	private:
		virtual void* GetUnderlyingHandle() const = 0;
	};

}
