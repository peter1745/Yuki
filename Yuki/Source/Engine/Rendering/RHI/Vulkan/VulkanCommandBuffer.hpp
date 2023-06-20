#pragma once

#include "Rendering/RHI/CommandBuffer.hpp"
#include "VulkanRenderContext.hpp"

namespace Yuki {

	class VulkanCommandBufferPool;

	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		void Begin() override;
		void End() override;

		void BindVertexBuffer(Buffer* InVertexBuffer) override;
		void BindIndexBuffer(Buffer* InIndexBuffer) override;
		void BindPipeline(GraphicsPipeline* InPipeline) override;

		void SetViewport(Viewport* InViewport) override;

		void BeginRendering(RenderTarget* InRenderTarget);
		void BeginRendering(Viewport* InViewport);
		void EndRendering();

		void Draw(uint32_t InVertexCount, uint32_t InInstanceCount, uint32_t InFirstVertex, uint32_t InFirstInstance) override;
		void DrawIndexed(uint32_t InIndexCount, uint32_t InInstanceCount, uint32_t InFirstIndex, int32_t InVertexOffset, uint32_t InFirstInstance) override;

	private:
		VulkanCommandBuffer(VulkanRenderContext* InContext, VulkanCommandBufferPool* InCommandPool);

		void* GetUnderlyingHandle() const override { return m_CommandBuffer; }

	private:
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

	private:
		friend class VulkanCommandBufferPool;
	};

}
