#pragma once

#include "RHI/RenderContext.hpp"
#include "RHI/Image2D.hpp"

namespace Yuki {

	struct RenderTargetInfo
	{
		static constexpr size_t MaxColorAttachments = 8;

		uint32_t Width = 0;
		uint32_t Height = 0;

		std::array<ImageFormat, MaxColorAttachments> ColorAttachments;
		ImageFormat DepthAttachmentFormat = ImageFormat::None;

		RenderTargetInfo()
		{
			ColorAttachments.fill(ImageFormat::None);
		}
	};

	class RenderTarget
	{
	private:
		static RenderTarget* Create(RenderContext* InContext, const RenderTargetInfo& InInfo);
		static void Destroy(RenderContext* InContext, RenderTarget* InRenderTarget);

	private:
		struct Attachment
		{
			Image2D* Image = nullptr;
			ImageView2D* ImageView = nullptr;

			bool IsValid() const { return Image != nullptr && ImageView != nullptr; }
		};

	private:
		RenderTargetInfo m_Info;

		std::array<Attachment, RenderTargetInfo::MaxColorAttachments> m_ColorAttachments;
		Attachment m_DepthAttachment;

	private:
		// TODO(Peter): I'd rather not expose VulkanRenderContext here, but for now it's fine
		friend class VulkanRenderContext;
	};

}
