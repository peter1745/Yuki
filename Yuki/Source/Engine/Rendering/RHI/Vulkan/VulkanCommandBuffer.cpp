#include "VulkanCommandBuffer.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanGraphicsPipeline.hpp"

namespace Yuki {

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanRenderContext* InContext, VkCommandPool InCommandPool)
	{
		VkCommandBufferAllocateInfo allocateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = InCommandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		vkAllocateCommandBuffers(InContext->GetDevice(), &allocateInfo, &m_CommandBuffer);
	}

	void VulkanCommandBuffer::Begin()
	{
		VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
	}

	void VulkanCommandBuffer::End()
	{
		vkEndCommandBuffer(m_CommandBuffer);
	}

	void VulkanCommandBuffer::BindVertexBuffer(Buffer* InVertexBuffer)
	{
		YUKI_VERIFY(InVertexBuffer->GetInfo().Type == BufferType::VertexBuffer);
		VkBuffer buffer = static_cast<VulkanBuffer*>(InVertexBuffer)->GetVkBuffer();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &buffer, &offset);
	}

	void VulkanCommandBuffer::BindPipeline(GraphicsPipeline* InPipeline)
	{
		vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VulkanGraphicsPipeline*>(InPipeline)->Pipeline);
	}

	void VulkanCommandBuffer::SetViewport(Viewport* InViewport)
	{
		VkViewport viewport =
		{
			.x = 0.0f,
			.y = 0.0f,
			.width = float(InViewport->GetWidth()),
			.height = float(InViewport->GetHeight()),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);

		VkRect2D scissor =
		{
			.offset = { .x = 0, .y = 0, },
			.extent = {
				.width = InViewport->GetWidth(),
				.height = InViewport->GetHeight(),
			},
		};
		vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
	}

	void VulkanCommandBuffer::Draw(uint32_t InVertexCount, uint32_t InInstanceCount, uint32_t InFirstVertex, uint32_t InFirstInstance)
	{
		vkCmdDraw(m_CommandBuffer, InVertexCount, InInstanceCount, InFirstVertex, InFirstInstance);
	}

}
