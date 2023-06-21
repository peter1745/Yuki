#include "VulkanCommandBufferPool.hpp"
#include "VulkanCommandBuffer.hpp"

namespace Yuki {

	CommandBuffer* VulkanCommandBufferPool::NewCommandBuffer()
	{
		if (m_CommandBufferIndex >= m_CommandBuffers.size())
		{
			Unique<VulkanCommandBuffer> commandBuffer(new VulkanCommandBuffer(m_Context, this));
			m_CommandBuffers.emplace_back(std::move(commandBuffer));
		}

		return m_CommandBuffers[m_CommandBufferIndex++].GetPtr();
	}

	void VulkanCommandBufferPool::Reset()
	{
		vkResetCommandPool(m_Context->GetDevice(), m_CommandPool, 0);
		m_CommandBufferIndex = 0;
	}

	VulkanCommandBufferPool::VulkanCommandBufferPool(VulkanRenderContext* InContext, CommandBufferPoolInfo InInfo)
		: m_Context(InContext)
	{
		VkCommandPoolCreateInfo poolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = InInfo.IsTransient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : VkCommandPoolCreateFlags(0),
			.queueFamilyIndex = static_cast<VulkanQueue*>(InContext->GetGraphicsQueue())->GetFamilyIndex()
		};
		vkCreateCommandPool(InContext->GetDevice(), &poolInfo, nullptr, &m_CommandPool);
	}

	VulkanCommandBufferPool::~VulkanCommandBufferPool()
	{
		vkDestroyCommandPool(m_Context->GetDevice(), m_CommandPool, nullptr);
	}

}
