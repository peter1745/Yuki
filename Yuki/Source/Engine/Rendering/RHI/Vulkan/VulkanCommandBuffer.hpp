#pragma once

#include "Rendering/RHI/CommandBuffer.hpp"
#include "VulkanRenderContext.hpp"

namespace Yuki {

	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		void Begin() override;
		void End() override;

		void BindVertexBuffer(Buffer* InVertexBuffer) override;
		void BindPipeline(GraphicsPipeline* InPipeline) override;

		void SetViewport(Viewport* InViewport) override;

		void Draw(uint32_t InVertexCount, uint32_t InInstanceCount, uint32_t InFirstVertex, uint32_t InFirstInstance) override;

	private:
		VulkanCommandBuffer(VulkanRenderContext* InContext, VkCommandPool InCommandPool);

		void* GetUnderlyingHandle() const override { return m_CommandBuffer; }

	private:
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

	private:
		friend class VulkanRenderContext;
	};

}
