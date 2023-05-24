#pragma once

#include "RenderContext.hpp"
#include "Image2D.hpp"

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
	public:
		struct AttachmentInfo
		{
			Image2D* Image = nullptr;
			ImageView2D* ImageView = nullptr;

			bool IsValid() const { return Image != nullptr && ImageView != nullptr; }
		};

	public:
		virtual ~RenderTarget() = default;

		virtual const RenderTargetInfo& GetInfo() const = 0;

		virtual const std::vector<AttachmentInfo>& GetColorAttachmentInfos() const = 0;
		virtual const AttachmentInfo& GetDepthAttachmentInfo() const = 0;
	};

}
