#include "VulkanCommandPool.hpp"
#include "VulkanRenderContext.hpp"
#include "VulkanHelper.hpp"

#include "Math/Math.hpp"

namespace Yuki {

	CommandPool VulkanRenderContext::CreateCommandPool(Queue InQueue)
	{
		auto[handle, pool] = m_CommandPools.Acquire();	

		auto& queue = m_Queues.Get(InQueue);
		
		VkCommandPoolCreateInfo poolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VkCommandPoolCreateFlags(0),
			.queueFamilyIndex = queue.FamilyIndex,
		};
		vkCreateCommandPool(m_LogicalDevice, &poolInfo, nullptr, &pool.Pool);

		return handle;
	}

	void VulkanRenderContext::CommandPoolReset(CommandPool InCommandPool)
	{
		auto& pool = m_CommandPools.Get(InCommandPool);
		YUKI_VERIFY(vkResetCommandPool(m_LogicalDevice, pool.Pool, 0) == VK_SUCCESS);
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
			YUKI_VERIFY(vkAllocateCommandBuffers(m_LogicalDevice, &allocateInfo, &commandList.CommandBuffer) == VK_SUCCESS);

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
		YUKI_VERIFY(vkBeginCommandBuffer(commandList.CommandBuffer, &beginInfo) == VK_SUCCESS);
	}

	void VulkanRenderContext::CommandListEnd(CommandList InCommandList)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		YUKI_VERIFY(vkEndCommandBuffer(commandList.CommandBuffer) == VK_SUCCESS);
	}

	void VulkanRenderContext::CommandListBeginRendering(CommandList InCommandList, Swapchain InSwapchain)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		auto& swapchain = m_Swapchains.Get(InSwapchain);

		CommandListTransitionImage(InCommandList, swapchain.Images[swapchain.CurrentImage], ImageLayout::Attachment);

		VkClearColorValue clearColor = { 0.05f, 0.05f, 0.05f, 1.0f };

		VkViewport viewport =
		{
			.x = 0,
			.y = 0,
			.width = float(swapchain.Width),
			.height = float(swapchain.Height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
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

		auto& depthAttachmentImageView = m_ImageViews.Get(m_Images.Get(swapchain.DepthImage).DefaultImageView);
		VkClearDepthStencilValue depthClearValue = { .depth = 0.0f };
		VkRenderingAttachmentInfo depthAttachmentInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = nullptr,
			.imageView = depthAttachmentImageView.ImageView,
			.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {
				.depthStencil = depthClearValue
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
			.pDepthAttachment = &depthAttachmentInfo,
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

	void VulkanRenderContext::CommandListBindDescriptorSet(CommandList InCommandList, Pipeline InPipeline, DescriptorSet InSet)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		auto& pipeline = m_Pipelines.Get(InPipeline);
		auto& set = m_DescriptorSets.Get(InSet);
		vkCmdBindDescriptorSets(commandList.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Layout, 0, 1, &set.Handle, 0, nullptr);
	}

	void VulkanRenderContext::CommandListPushConstants(CommandList InCommandList, Pipeline InPipeline, const void* InData, uint32_t InDataSize, uint32_t InOffset)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		auto& pipeline = m_Pipelines.Get(InPipeline);
		vkCmdPushConstants(commandList.CommandBuffer, pipeline.Layout, VK_SHADER_STAGE_ALL, InOffset, InDataSize, InData);
	}

	void VulkanRenderContext::CommandListTransitionImage(CommandList InCommandList, Image InImage, ImageLayout InNewLayout)
	{
		auto& image = m_Images.Get(InImage);
		auto& commandList = m_CommandLists.Get(InCommandList);
		auto newLayout = VulkanHelper::ImageLayoutToVkImageLayout(InNewLayout);

		// TODO(Peter): Mips / Array Levels

		VkImageMemoryBarrier2 barrier =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = image.PipelineStage,
			.srcAccessMask = image.AccessFlags,
			.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			.dstAccessMask = 0,
			.oldLayout = image.Layout,
			.newLayout = newLayout,
			.image = image.Image,
			.subresourceRange = {
				.aspectMask = image.AspectFlags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};

		VkDependencyInfo dependencyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier,
		};

		vkCmdPipelineBarrier2(commandList.CommandBuffer, &dependencyInfo);

		image.Layout = newLayout;
		image.PipelineStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		image.AccessFlags = 0;
	}

	void VulkanRenderContext::CommandListCopyToBuffer(CommandList InCommandList, Buffer InDstBuffer, uint32_t InDstOffset, Buffer InSrcBuffer, uint32_t InSrcOffset, uint32_t InSize)
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
	
	void VulkanRenderContext::CommandListCopyToImage(CommandList InCommandList, Image InDstImage, Buffer InSrcBuffer, uint32_t InSrcOffset)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		auto& image = m_Images.Get(InDstImage);
		auto& srcBuffer = m_Buffers.Get(InSrcBuffer);

		auto prevImageLayout = VulkanHelper::VkImageLayoutToImageLayout(image.Layout);

		CommandListTransitionImage(InCommandList, InDstImage, ImageLayout::TransferDestination);

		VkBufferImageCopy2 region =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
			.pNext = nullptr,
			.bufferOffset = InSrcOffset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = image.AspectFlags,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { image.Width, image.Height, 1 },
		};

		VkCopyBufferToImageInfo2 copyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
			.pNext = nullptr,
			.srcBuffer = srcBuffer.Handle,
			.dstImage = image.Image,
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.regionCount = 1,
			.pRegions = &region,
		};

		vkCmdCopyBufferToImage2(commandList.CommandBuffer, &copyInfo);

		CommandListTransitionImage(InCommandList, InDstImage, prevImageLayout);
	}

	void VulkanRenderContext::CommandListBlitImage(CommandList InCommandList, Image InDstImage, Image InSrcImage)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		auto& dstImage = m_Images.Get(InDstImage);
		auto& srcImage = m_Images.Get(InSrcImage);
		
		auto dstImageLayout = VulkanHelper::VkImageLayoutToImageLayout(dstImage.Layout);
		auto srcImageLayout = VulkanHelper::VkImageLayoutToImageLayout(srcImage.Layout);

		CommandListTransitionImage(InCommandList, InDstImage, ImageLayout::TransferDestination);
		CommandListTransitionImage(InCommandList, InSrcImage, ImageLayout::TransferSource);

		VkImageBlit2 imageBlit =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
			.pNext = nullptr,
			.srcSubresource = {
				.aspectMask = srcImage.AspectFlags,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffsets = {
				{ 0, 0, 0 },
				{ int32_t(srcImage.Width), int32_t(srcImage.Height), 1 }
			},
			.dstSubresource = {
				.aspectMask = dstImage.AspectFlags,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffsets = {
				{ 0, 0, 0 },
				{ int32_t(dstImage.Width), int32_t(dstImage.Height), 1 }
			},
		};

		VkBlitImageInfo2 blitImageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
			.pNext = nullptr,
			.srcImage = srcImage.Image,
			.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.dstImage = dstImage.Image,
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.regionCount = 1,
			.pRegions = &imageBlit,
			.filter = VK_FILTER_LINEAR,
		};

		vkCmdBlitImage2(commandList.CommandBuffer, &blitImageInfo);

		CommandListTransitionImage(InCommandList, InDstImage, dstImageLayout);
		CommandListTransitionImage(InCommandList, InSrcImage, srcImageLayout);
	}

	void VulkanRenderContext::CommandListDraw(CommandList InCommandList, uint32_t InVertexCount)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		vkCmdDraw(commandList.CommandBuffer, InVertexCount, 1, 0, 0);
	}

	void VulkanRenderContext::CommandListDrawIndexed(CommandList InCommandList, uint32_t InIndexCount)
	{
		auto& commandList = m_CommandLists.Get(InCommandList);
		vkCmdDrawIndexed(commandList.CommandBuffer, InIndexCount, 1, 0, 0, 0);
	}

	void VulkanRenderContext::CommandListPrepareSwapchainPresent(CommandList InCommandList, Swapchain InSwapchain)
	{
		auto& swapchain = m_Swapchains.Get(InSwapchain);
		CommandListTransitionImage(InCommandList, swapchain.Images[swapchain.CurrentImage], ImageLayout::Present);
	}

}
