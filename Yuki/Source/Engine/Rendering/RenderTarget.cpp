#include "Rendering/RenderTarget.hpp"

namespace Yuki {

	RenderTarget* RenderTarget::Create(RenderContext* InContext, const RenderTargetInfo& InInfo)
	{
		RenderTarget* renderTarget = new RenderTarget();
		renderTarget->m_Info = InInfo;

		for (uint32_t i = 0; i < RenderTargetInfo::MaxColorAttachments; i++)
		{
			if (InInfo.ColorAttachments[i] == ImageFormat::None)
				continue;

			Image2D* colorAttachmentImage = InContext->CreateImage2D(InInfo.Width, InInfo.Height, InInfo.ColorAttachments[i]);

			renderTarget->m_ColorAttachments[i].Image = colorAttachmentImage;
			renderTarget->m_ColorAttachments[i].ImageView = InContext->CreateImageView2D(colorAttachmentImage);
		}

		if (InInfo.DepthAttachmentFormat != ImageFormat::None)
		{
			renderTarget->m_DepthAttachment.Image = InContext->CreateImage2D(InInfo.Width, InInfo.Height, InInfo.DepthAttachmentFormat);
			renderTarget->m_DepthAttachment.ImageView = InContext->CreateImageView2D(renderTarget->m_DepthAttachment.Image);
		}

		return renderTarget;
	}

	void RenderTarget::Destroy(RenderContext* InContext, RenderTarget* InRenderTarget)
	{
		for (uint32_t i = 0; i < RenderTargetInfo::MaxColorAttachments; i++)
		{
			if (!InRenderTarget->m_ColorAttachments[i].IsValid())
				continue;

			InContext->DestroyImageView2D(InRenderTarget->m_ColorAttachments[i].ImageView);
			InContext->DestroyImage2D(InRenderTarget->m_ColorAttachments[i].Image);
		}

		if (InRenderTarget->m_DepthAttachment.IsValid())
		{
			InContext->DestroyImageView2D(InRenderTarget->m_DepthAttachment.ImageView);
			InContext->DestroyImage2D(InRenderTarget->m_DepthAttachment.Image);
		}

		delete InRenderTarget;
	}

}
