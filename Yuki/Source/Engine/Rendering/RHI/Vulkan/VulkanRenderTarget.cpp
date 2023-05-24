#include "VulkanRenderTarget.hpp"
#include "VulkanImage2D.hpp"

namespace Yuki {

	RenderTarget* VulkanRenderTarget::Create(RenderContext* InContext, const RenderTargetInfo& InInfo)
	{
		VulkanRenderTarget* renderTarget = new VulkanRenderTarget();
		renderTarget->m_Info = InInfo;

		for (uint32_t i = 0; i < RenderTargetInfo::MaxColorAttachments; i++)
		{
			if (InInfo.ColorAttachments[i] == ImageFormat::None)
				continue;

			VulkanImage2D* image = (VulkanImage2D*)InContext->CreateImage2D(InInfo.Width, InInfo.Height, InInfo.ColorAttachments[i]);
			VulkanImageView2D* imageView = (VulkanImageView2D*)InContext->CreateImageView2D(image);

			auto& attachmentInfo = renderTarget->m_ColorAttachmentInfos.emplace_back();
			attachmentInfo.Image = image;
			attachmentInfo.ImageView = imageView;

			VkClearValue clearColor = {};
			clearColor.color.float32[0] = 1.0f;
			clearColor.color.float32[1] = 0.0f;
			clearColor.color.float32[2] = 0.0f;
			clearColor.color.float32[3] = 1.0f;

			auto& colorAttachment = renderTarget->m_ColorAttachments.emplace_back();
			colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			colorAttachment.imageView = imageView->GetVkImageView();
			colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.clearValue = clearColor;
		}

		if (InInfo.DepthAttachmentFormat != ImageFormat::None)
		{
			VulkanImage2D* image = (VulkanImage2D*)InContext->CreateImage2D(InInfo.Width, InInfo.Height, InInfo.DepthAttachmentFormat);
			VulkanImageView2D* imageView = (VulkanImageView2D*)InContext->CreateImageView2D(image);

			renderTarget->m_DepthAttachmentInfo.Image = image;
			renderTarget->m_DepthAttachmentInfo.ImageView = imageView;

			VkClearValue depthClearValue = {
				.depthStencil = {
					.depth = 1.0f,
				},
			};

			renderTarget->m_DepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			renderTarget->m_DepthAttachment.imageView = imageView->GetVkImageView();
			renderTarget->m_DepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			renderTarget->m_DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			renderTarget->m_DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			renderTarget->m_DepthAttachment.clearValue = depthClearValue;
		}

		return renderTarget;
	}

	void VulkanRenderTarget::Destroy(RenderContext* InContext, VulkanRenderTarget* InRenderTarget)
	{
		for (uint32_t i = 0; i < InRenderTarget->m_ColorAttachmentInfos.size(); i++)
		{
			InContext->DestroyImageView2D(InRenderTarget->m_ColorAttachmentInfos[i].ImageView);
			InContext->DestroyImage2D(InRenderTarget->m_ColorAttachmentInfos[i].Image);
		}

		if (InRenderTarget->m_DepthAttachmentInfo.IsValid())
		{
			InContext->DestroyImageView2D(InRenderTarget->m_DepthAttachmentInfo.ImageView);
			InContext->DestroyImage2D(InRenderTarget->m_DepthAttachmentInfo.Image);
		}

		delete InRenderTarget;
	}

}
