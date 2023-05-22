#include "Rendering/RenderTarget.hpp"

namespace Yuki {

	RenderTarget::RenderTarget(RenderContext* InContext, const RenderTargetInfo& InInfo)
	    : m_Info(InInfo)
	{
		for (uint32_t i = 0; i < RenderTargetInfo::MaxColorAttachments; i++)
		{
			if (m_Info.ColorAttachments[i] == ImageFormat::None)
				continue;

			m_ColorAttachments[i].Image = InContext->CreateImage2D(m_Info.Width, m_Info.Height, m_Info.ColorAttachments[i]);
			m_ColorAttachments[i].ImageView = InContext->CreateImageView2D(m_ColorAttachments[i].Image);
		}
	}

}
