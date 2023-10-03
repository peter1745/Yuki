#pragma once

#include "Engine/Containers/Span.hpp"

#include "RenderHandles.hpp"

namespace Yuki::RHI {

	enum class AttachmentLoadOp { Load, Clear, DontCare };
	enum class AttachmentStoreOp { Store, DontCare };

	struct RenderTargetAttachment
	{
		ImageViewRH ImageView;
		AttachmentLoadOp LoadOp;
		AttachmentStoreOp StoreOp;
	};

	struct RenderTarget
	{
		Span<RenderTargetAttachment> ColorAttachments;
	};

}
