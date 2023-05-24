#include "VulkanRenderTarget.hpp"
#include "VulkanImage2D.hpp"

namespace Yuki {

	VulkanRenderTarget::VulkanRenderTarget(VulkanRenderContext* InContext, const RenderTargetInfo& InInfo)
		: m_Context(InContext), m_Info(InInfo)
	{
		for (uint32_t i = 0; i < RenderTargetInfo::MaxColorAttachments; i++)
		{
			if (InInfo.ColorAttachments[i] == ImageFormat::None)
				continue;

			VulkanImage2D* image = (VulkanImage2D*)InContext->CreateImage2D(InInfo.Width, InInfo.Height, InInfo.ColorAttachments[i]);
			VulkanImageView2D* imageView = (VulkanImageView2D*)InContext->CreateImageView2D(image);

			auto& attachmentInfo = m_ColorAttachmentInfos.emplace_back();
			attachmentInfo.Image = image;
			attachmentInfo.ImageView = imageView;

			VkClearValue clearColor = {};
			clearColor.color.float32[0] = 1.0f;
			clearColor.color.float32[1] = 0.0f;
			clearColor.color.float32[2] = 0.0f;
			clearColor.color.float32[3] = 1.0f;

			auto& colorAttachment = m_ColorAttachments.emplace_back();
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachment.imageView = imageView->GetVkImageView();
			colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.clearValue = clearColor;
		}

		if (InInfo.DepthAttachmentFormat != ImageFormat::None)
		{
			auto* image = (VulkanImage2D*)InContext->CreateImage2D(InInfo.Width, InInfo.Height, InInfo.DepthAttachmentFormat);
			auto* imageView = (VulkanImageView2D*)InContext->CreateImageView2D(image);

			m_DepthAttachmentInfo.Image = image;
			m_DepthAttachmentInfo.ImageView = imageView;

			VkClearValue depthClearValue = {
				.depthStencil = {
					.depth = 1.0f,
				},
			};

			m_DepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			m_DepthAttachment.imageView = imageView->GetVkImageView();
			m_DepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			m_DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			m_DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			m_DepthAttachment.clearValue = depthClearValue;
		}
	}

	VulkanRenderTarget::~VulkanRenderTarget()
	{
		for (uint32_t i = 0; i < m_ColorAttachmentInfos.size(); i++)
		{
			m_Context->DestroyImageView2D(m_ColorAttachmentInfos[i].ImageView);
			m_Context->DestroyImage2D(m_ColorAttachmentInfos[i].Image);
		}

		if (m_DepthAttachmentInfo.IsValid())
		{
			m_Context->DestroyImageView2D(m_DepthAttachmentInfo.ImageView);
			m_Context->DestroyImage2D(m_DepthAttachmentInfo.Image);
		}
	}

}
