#include "VulkanCommandBuffer.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanImage2D.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanCommandBufferPool.hpp"
#include "VulkanDescriptorSet.hpp"
#include "VulkanHelper.hpp"

#include "Rendering/RHI/RenderTarget.hpp"

#include "Math/Math.hpp"

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

	void VulkanCommandBuffer::BindDescriptorSets(GraphicsPipeline* InPipeline, std::span<DescriptorSet* const> InDescriptorSets)
	{
		auto pipelineLayout = static_cast<VulkanGraphicsPipeline*>(InPipeline)->Layout;

		List<VkDescriptorSet> descriptorSets;
		descriptorSets.reserve(InDescriptorSets.size());
		for (auto* set : InDescriptorSets)
			descriptorSets.emplace_back(static_cast<VulkanDescriptorSet*>(set)->m_Set);

		vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, uint32_t(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
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

		VulkanHelper::TransitionImage(m_CommandBuffer, image->GetVkImage(), image->m_CurrentPipelineStage, image->m_CurrentAccessFlags, image->m_CurrentLayout, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0, imageLayout, aspectFlags);

		image->m_CurrentPipelineStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		image->m_CurrentAccessFlags = 0;
		image->m_CurrentLayout = imageLayout;
	}

	void VulkanCommandBuffer::CopyToBuffer(Buffer* InDstBuffer, uint32_t InDstOffset, Buffer* InSrcBuffer, uint32_t InSrcOffset, uint32_t InSize)
	{
		uint32_t size = InSize;
		if (size == 0)
			size = Math::Min(InDstBuffer->GetInfo().Size - InDstOffset, InSrcBuffer->GetInfo().Size - InSrcOffset);

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
			.srcBuffer = static_cast<VulkanBuffer*>(InSrcBuffer)->GetVkBuffer(),
			.dstBuffer = static_cast<VulkanBuffer*>(InDstBuffer)->GetVkBuffer(),
			.regionCount = 1,
			.pRegions = &copyRegion
		};

		vkCmdCopyBuffer2(m_CommandBuffer, &bufferCopyInfo);
	}

	void VulkanCommandBuffer::CopyToImage(Image2D* InDstImage, Buffer* InSrcBuffer, uint32_t InSrcOffset)
	{
		VkImageAspectFlags aspectMask = IsDepthFormat(InDstImage->GetImageFormat()) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		{
			auto* image = static_cast<VulkanImage2D*>(InDstImage);
			VulkanHelper::TransitionImage(m_CommandBuffer, image->GetVkImage(), image->m_CurrentPipelineStage, image->m_CurrentAccessFlags, image->m_CurrentLayout,
											VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, aspectMask);
		}

		VkBufferImageCopy2 region =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
			.pNext = nullptr,
			.bufferOffset = InSrcOffset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = aspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { InDstImage->GetWidth(), InDstImage->GetHeight(), 1 },
		};

		VkCopyBufferToImageInfo2 copyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
			.pNext = nullptr,
			.srcBuffer = static_cast<VulkanBuffer*>(InSrcBuffer)->GetVkBuffer(),
			.dstImage = static_cast<VulkanImage2D*>(InDstImage)->GetVkImage(),
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.regionCount = 1,
			.pRegions = &region,
		};

		vkCmdCopyBufferToImage2(m_CommandBuffer, &copyInfo);

		{
			auto* image = static_cast<VulkanImage2D*>(InDstImage);
			VulkanHelper::TransitionImage(m_CommandBuffer, image->GetVkImage(), VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, image->m_CurrentPipelineStage, image->m_CurrentAccessFlags, image->m_CurrentLayout, aspectMask);
		}
	}

	void VulkanCommandBuffer::BlitImage(Image2D* InDstImage, Image2D* InSrcImage)
	{
		VkImageAspectFlags srcAspectMask = IsDepthFormat(InSrcImage->GetImageFormat()) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		VkImageAspectFlags dstAspectMask = IsDepthFormat(InDstImage->GetImageFormat()) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		{
			auto* dstImage = static_cast<VulkanImage2D*>(InDstImage);
			VulkanHelper::TransitionImage(m_CommandBuffer, dstImage->GetVkImage(), dstImage->m_CurrentPipelineStage, dstImage->m_CurrentAccessFlags, dstImage->m_CurrentLayout, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstAspectMask);
			auto* srcImage = static_cast<VulkanImage2D*>(InSrcImage);
			VulkanHelper::TransitionImage(m_CommandBuffer, srcImage->GetVkImage(), srcImage->m_CurrentPipelineStage, srcImage->m_CurrentAccessFlags, srcImage->m_CurrentLayout, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstAspectMask);
		}

		VkImageBlit2 imageBlit =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
			.pNext = nullptr,
			.srcSubresource = {
				.aspectMask = srcAspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffsets = {
				{ 0, 0, 0 },
				{ int32_t(InSrcImage->GetWidth()), int32_t(InSrcImage->GetHeight()), 1 }
			},
			.dstSubresource = {
				.aspectMask = dstAspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffsets = {
				{ 0, 0, 0 },
				{ int32_t(InDstImage->GetWidth()), int32_t(InDstImage->GetHeight()), 1 }
			},
		};

		VkBlitImageInfo2 blitImageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
			.pNext = nullptr,
			.srcImage = static_cast<VulkanImage2D*>(InSrcImage)->GetVkImage(),
			.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.dstImage = static_cast<VulkanImage2D*>(InDstImage)->GetVkImage(),
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.regionCount = 1,
			.pRegions = &imageBlit,
			.filter = VK_FILTER_LINEAR,
		};

		vkCmdBlitImage2(m_CommandBuffer, &blitImageInfo);

		{
			auto* dstImage = static_cast<VulkanImage2D*>(InDstImage);
			VulkanHelper::TransitionImage(m_CommandBuffer, dstImage->GetVkImage(), VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstImage->m_CurrentPipelineStage, dstImage->m_CurrentAccessFlags, dstImage->m_CurrentLayout, dstAspectMask);
			auto* srcImage = static_cast<VulkanImage2D*>(InSrcImage);
			VulkanHelper::TransitionImage(m_CommandBuffer, srcImage->GetVkImage(), VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, 0, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcImage->m_CurrentPipelineStage, srcImage->m_CurrentAccessFlags, srcImage->m_CurrentLayout, dstAspectMask);
		}
	}

}
