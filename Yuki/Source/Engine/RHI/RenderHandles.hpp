#pragma once

#include "Engine/Common/Core.hpp"

namespace Yuki::RHI {

#define YUKI_RENDER_HANDLE(Type) enum class Type##RH{}

	YUKI_RENDER_HANDLE(Queue);
	YUKI_ENUM(QueueType)
	{
		Graphics = 1 << 0,
		Compute  = 1 << 1,
		Transfer = 1 << 2
	};

	YUKI_RENDER_HANDLE(Swapchain);
	YUKI_RENDER_HANDLE(Fence);
	YUKI_RENDER_HANDLE(CommandPool);
	YUKI_RENDER_HANDLE(CommandList);

	YUKI_RENDER_HANDLE(Image);
	YUKI_RENDER_HANDLE(ImageView);
	enum class ImageFormat
	{
		RGBA8,
		BGRA8,
		D32SFloat
	};
	YUKI_ENUM(ImageUsage)
	{
		ColorAttachment = 1 << 0,
		DepthAttachment = 1 << 1,
		Sampled			= 1 << 2,
		TransferDest	= 1 << 3,
		TransferSource	= 1 << 4,
	};
	enum class ImageLayout
	{
		Undefined = -1,
		Attachment,
		ShaderReadOnly,
		Present,
		TransferDest,
		TransferSource
	};

#undef YUKI_RENDER_HANDLE
}
