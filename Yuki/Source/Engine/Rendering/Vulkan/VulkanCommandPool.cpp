#include "VulkanCommandPool.hpp"
#include "VulkanRenderContext.hpp"

#include "Math/Math.hpp"

namespace Yuki {

	CommandPool VulkanRenderContext::CreateCommandPool()
	{
		auto[handle, pool] = m_CommandPools.Acquire();	
		
		VkCommandPoolCreateInfo poolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VkCommandPoolCreateFlags(0),
			.queueFamilyIndex = m_Queues.Get(m_GraphicsQueue).FamilyIndex, // TODO(Peter): Pass queue as parameter (compute, transfer, etc...)
		};
		vkCreateCommandPool(m_LogicalDevice, &poolInfo, nullptr, &pool.Pool);

		return handle;
	}

	void VulkanRenderContext::CommandPoolReset(CommandPool InCommandPool)
	{
		auto& pool = m_CommandPools.Get(InCommandPool);
		vkResetCommandPool(m_LogicalDevice, pool.Pool, 0);
		pool.NextList = 0;
	}

	void VulkanRenderContext::Destroy(CommandPool InCommandPool)
	{
		auto& pool = m_CommandPools.Get(InCommandPool);
		vkDestroyCommandPool(m_LogicalDevice, pool.Pool, nullptr);
		m_CommandPools.Return(InCommandPool);
	}

	CommandList VulkanRenderContext::CreateCommandList(CommandPool InCommandPool)
	{
		auto& pool = m_CommandPools.Get(InCommandPool);

		if (pool.NextList >= pool.AllocatedLists.size())
		{
			auto[handle, commandList] = m_CommandLists.Acquire();
			
			VkCommandBufferAllocateInfo allocateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = nullptr,
				.commandPool = pool.Pool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};
			vkAllocateCommandBuffers(m_LogicalDevice, &allocateInfo, &commandList.CommandBuffer);

			pool.AllocatedLists.emplace_back(handle);
		}

		return pool.AllocatedLists[pool.NextList++];
	}

	void VulkanRenderContext::CommandListBegin(CommandList InCommandList)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);

		VkCommandBufferBeginInfo beginInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr,
		};
		vkBeginCommandBuffer(commandList.CommandBuffer, &beginInfo);
	}

	void VulkanRenderContext::CommandListEnd(CommandList InCommandList)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		vkEndCommandBuffer(commandList.CommandBuffer);
	}

	void VulkanRenderContext::CommandListBeginRendering(CommandList InCommandList, Swapchain InSwapchain)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		auto& swapchain = m_Swapchains.Get(InSwapchain);

		ImageTransition(InCommandList, swapchain.Images[swapchain.CurrentImage], ImageLayout::Attachment);

		VkClearColorValue clearColor = { 1.0f, 0.0f, 0.0f, 1.0f };

		VkViewport viewport =
		{
			.x = 0,
			.y = 0,
			.width = float(swapchain.Width),
			.height = float(swapchain.Height),
			.minDepth = 0.0f,
			.maxDepth = 0.0f,
		};
		vkCmdSetViewport(commandList.CommandBuffer, 0, 1, &viewport);

		VkRect2D scissor =
		{
			.offset = { 0, 0 },
			.extent = { swapchain.Width, swapchain.Height }
		};
		vkCmdSetScissor(commandList.CommandBuffer, 0, 1, &scissor);

		auto& colorAttachmentImageView = m_ImageViews.Get(swapchain.ImageViews[swapchain.CurrentImage]);
		VkRenderingAttachmentInfo colorAttachmentInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = nullptr,
			.imageView = colorAttachmentImageView.ImageView,
			.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {
				.color = clearColor,
			},
		};

		VkRenderingInfo renderingInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderArea = {
				.offset = { 0, 0 },
				.extent = { swapchain.Width, swapchain.Height }
			},
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo,
			.pDepthAttachment = nullptr,
			.pStencilAttachment = nullptr,
		};
		vkCmdBeginRendering(commandList.CommandBuffer, &renderingInfo);
	}

	void VulkanRenderContext::CommandListEndRendering(CommandList InCommandList)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		vkCmdEndRendering(commandList.CommandBuffer);
	}

	void VulkanRenderContext::CommandListBindPipeline(CommandList InCommandList, Pipeline InPipeline)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		auto& pipeline = m_Pipelines.Get(InPipeline);
		vkCmdBindPipeline(commandList.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
	}

	void VulkanRenderContext::CommandListBindBuffer(CommandList InCommandList, Buffer InBuffer)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		auto& buffer = m_Buffers.Get(InBuffer);

		VkDeviceSize offset = 0;
		switch (buffer.Type)
		{
		case BufferType::VertexBuffer:
		{
			vkCmdBindVertexBuffers(commandList.CommandBuffer, 0, 1, &buffer.Handle, &offset);
			break;
		}
		case BufferType::IndexBuffer:
		{
			vkCmdBindIndexBuffer(commandList.CommandBuffer, buffer.Handle, offset, VK_INDEX_TYPE_UINT32);
		}
		}
	}

	void VulkanRenderContext::CommandListCopyBuffer(CommandList InCommandList, Buffer InDstBuffer, uint32_t InDstOffset, Buffer InSrcBuffer, uint32_t InSrcOffset, uint32_t InSize)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		auto& dstBuffer = m_Buffers.Get(InDstBuffer);
		auto& srcBuffer = m_Buffers.Get(InSrcBuffer);

		uint32_t size = InSize;
		if (size == 0)
			size = Math::Min(dstBuffer.Size - InDstOffset, srcBuffer.Size - InSrcOffset);

		VkBufferCopy2 copyRegion =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
			.srcOffset = InSrcOffset,
			.dstOffset = InDstOffset,
			.size = size,
		};

		VkCopyBufferInfo2 bufferCopyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
			.srcBuffer = srcBuffer.Handle,
			.dstBuffer = dstBuffer.Handle,
			.regionCount = 1,
			.pRegions = &copyRegion
		};

		vkCmdCopyBuffer2(commandList.CommandBuffer, &bufferCopyInfo);
	}

	void VulkanRenderContext::CommandListDraw(CommandList InCommandList, uint32_t InVertexCount)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		vkCmdDraw(commandList.CommandBuffer, InVertexCount, 1, 0, 0);
	}

}
