#pragma once

#include "Rendering/RHI/CommandBufferPool.hpp"

#include "VulkanRenderContext.hpp"
#include "VulkanCommandBuffer.hpp"

namespace Yuki {

	class VulkanCommandBufferPool : public CommandBufferPool
	{
	public:
		CommandBuffer* NewCommandBuffer() override;

		void Reset() override;

		VkCommandPool GetVkCommandPool() const { return m_CommandPool; }

	private:
		VulkanCommandBufferPool(VulkanRenderContext* InContext, CommandBufferPoolInfo InInfo);

	private:
		VulkanRenderContext* m_Context = nullptr;
		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
		List<Unique<VulkanCommandBuffer>> m_CommandBuffers;
		size_t m_CommandBufferIndex = 0;

	private:
		friend class VulkanRenderContext;
	};

}
