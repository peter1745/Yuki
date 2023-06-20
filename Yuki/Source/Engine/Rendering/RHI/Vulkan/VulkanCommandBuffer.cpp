#include "VulkanCommandBuffer.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanImage2D.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanCommandBufferPool.hpp"
#include "VulkanHelper.hpp"

#include "Rendering/RHI/RenderTarget.hpp"

namespace Yuki {

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanRenderContext* InContext, VulkanCommandBufferPool* InCommandPool)
	{
		VkCommandBufferAllocateInfo allocateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = InCommandPool->GetVkCommandPool(),
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

	void VulkanCommandBuffer::BindIndexBuffer(Buffer* InIndexBuffer)
	{
		YUKI_VERIFY(InIndexBuffer->GetInfo().Type == BufferType::IndexBuffer);
		VkBuffer buffer = static_cast<VulkanBuffer*>(InIndexBuffer)->GetVkBuffer();
		VkDeviceSize offset = 0;
		vkCmdBindIndexBuffer(m_CommandBuffer, buffer, offset, VK_INDEX_TYPE_UINT32);
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

	void VulkanCommandBuffer::BeginRendering(RenderTarget* InRenderTarget)
	{
		uint32_t colorAttachmentInfosCount = 0;
		std::array<VkRenderingAttachmentInfo, RenderTarget::MaxColorAttachments> colorAttachmentInfos;

		VkRenderingAttachmentInfo depthAttachmentInfo = { .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };

		VkClearValue clearColor = {};
		clearColor.color.float32[0] = 0.1f;
		clearColor.color.float32[1] = 0.1f;
		clearColor.color.float32[2] = 0.1f;
		clearColor.color.float32[3] = 1.0f;

		uint32_t width = 0, height = 0;

		for (auto* colorAttachment : InRenderTarget->ColorAttachments)
		{
			if (!colorAttachment)
				continue;

			TransitionImage(colorAttachment->GetImage(), ImageLayout::ColorAttachment);

			width = colorAttachment->GetImage()->GetWidth();
			height = colorAttachment->GetImage()->GetHeight();

			colorAttachmentInfos[colorAttachmentInfosCount++] =
			{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.imageView = static_cast<VulkanImageView2D*>(colorAttachment)->GetVkImageView(),
				.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue = clearColor,
			};
		}

		if (InRenderTarget->DepthAttachment)
		{
			TransitionImage(InRenderTarget->DepthAttachment->GetImage(), ImageLayout::DepthAttachment);

			VkClearValue depthClearValue = { .depthStencil = {.depth = 0.0f } };
			depthAttachmentInfo.imageView = static_cast<VulkanImageView2D*>(InRenderTarget->DepthAttachment)->GetVkImageView();
			depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachmentInfo.clearValue = depthClearValue;
		}

		VkRenderingInfo renderingInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea = {
				.offset = { 0, 0 },
				.extent = { width, height },
			},
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = colorAttachmentInfosCount,
			.pColorAttachments = colorAttachmentInfos.data(),
			.pDepthAttachment = &depthAttachmentInfo
		};

		vkCmdBeginRendering(m_CommandBuffer, &renderingInfo);
	}

	void VulkanCommandBuffer::BeginRendering(Viewport* InViewport)
	{
		RenderTarget viewportTarget =
		{
			.ColorAttachments = { InViewport->GetSwapchain()->GetCurrentImageView() },
			.DepthAttachment = InViewport->GetSwapchain()->GetDepthImage()->GetDefaultImageView()
		};

		BeginRendering(&viewportTarget);
	}

	void VulkanCommandBuffer::EndRendering()
	{
		vkCmdEndRendering(m_CommandBuffer);
	}

	void VulkanCommandBuffer::PushConstants(GraphicsPipeline* InPipeline, const void* InData, uint32_t InDataSize, uint32_t InOffset)
	{
		auto* pipeline = static_cast<VulkanGraphicsPipeline*>(InPipeline);
		vkCmdPushConstants(m_CommandBuffer, pipeline->Layout, VK_SHADER_STAGE_ALL, InOffset, InDataSize, InData);
	}

	void VulkanCommandBuffer::Draw(uint32_t InVertexCount, uint32_t InInstanceCount, uint32_t InFirstVertex, uint32_t InFirstInstance)
	{
		vkCmdDraw(m_CommandBuffer, InVertexCount, InInstanceCount, InFirstVertex, InFirstInstance);
	}

	void VulkanCommandBuffer::DrawIndexed(uint32_t InIndexCount, uint32_t InInstanceCount, uint32_t InFirstIndex, int32_t InVertexOffset, uint32_t InFirstInstance)
	{
		vkCmdDrawIndexed(m_CommandBuffer, InIndexCount, InInstanceCount, InFirstIndex, InVertexOffset, InFirstInstance);
	}

	void VulkanCommandBuffer::TransitionImage(Image2D* InImage, ImageLayout InNewLayout)
	{
		auto* image = static_cast<VulkanImage2D*>(InImage);

		VkImageAspectFlags aspectFlags = IsDepthFormat(InImage->GetImageFormat()) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageLayout imageLayout = VulkanHelper::ImageLayoutToVkImageLayout(InNewLayout);

		VkImageMemoryBarrier2 barrier =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = image->m_CurrentPipelineStage,
			.srcAccessMask = image->m_CurrentAccessFlags,
			.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, // TODO(Peter): Handle this properly (preferably in an abstracted way)
			.dstAccessMask = 0, // TODO(Peter): Handle this properly (preferably in an abstracted way)
			.oldLayout = image->m_CurrentLayout,
			.newLayout = imageLayout,
			.image = image->GetVkImage(),
			.subresourceRange = {
				.aspectMask = aspectFlags,
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

		vkCmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);

		image->m_CurrentPipelineStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		image->m_CurrentAccessFlags = 0;
		image->m_CurrentLayout = imageLayout;
	}

}
