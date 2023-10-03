#pragma once

#include "RenderHandles.hpp"
#include "RenderFeatures.hpp"
#include "RenderTarget.hpp"
#include "Barriers.hpp"

#include "Engine/Common/UniqueID.hpp"
#include "Engine/Containers/Span.hpp"

namespace Yuki {
	class WindowSystem;
}

namespace Yuki::RHI {

	class RenderDevice
	{
	public:
		virtual ~RenderDevice() = default;

		virtual bool IsFeatureEnabled(RendererFeature InFeature) const = 0;

	public:
		virtual QueueRH QueueRequest(QueueType InType) = 0;
		virtual void QueueAcquireImages(QueueRH InQueue, Span<SwapchainRH> InSwapchains, Span<FenceRH> InFences) = 0;
		virtual void QueueSubmit(QueueRH InQueue, Span<CommandListRH> InCommandLists, Span<FenceRH> InWaits, Span<FenceRH> InSignals) = 0;
		virtual void QueuePresent(QueueRH InQueue, Span<SwapchainRH> InSwapchains, Span<FenceRH> InFences) = 0;

		virtual SwapchainRH SwapchainCreate(const WindowSystem& InWindowSystem, UniqueID InWindowHandle) = 0;
		virtual ImageRH SwapchainGetCurrentImage(SwapchainRH InSwapchain) = 0;
		virtual ImageViewRH SwapchainGetCurrentImageView(SwapchainRH InSwapchain) = 0;
		virtual void SwapchainDestroy(SwapchainRH InSwapchain) = 0;

		virtual FenceRH FenceCreate() = 0;
		virtual void FenceWait(FenceRH InFence, uint64_t InValue = 0) = 0;
		virtual void FenceDestroy(FenceRH InFence) = 0;

		virtual CommandPoolRH CommandPoolCreate(QueueRH InQueue) = 0;
		virtual void CommandPoolReset(CommandPoolRH InPool) = 0;
		virtual CommandListRH CommandPoolNewList(CommandPoolRH InPool) = 0;
		virtual void CommandPoolDestroy(CommandPoolRH InPool) = 0;

		virtual void CommandListBegin(CommandListRH InList) = 0;
		virtual void CommandListImageBarrier(CommandListRH InList, ImageBarrier InBarrier) = 0;
		virtual void CommandListBeginRendering(CommandListRH InList, RenderTarget InRenderTarget) = 0;
		virtual void CommandListEndRendering(CommandListRH InList) = 0;
		virtual void CommandListEnd(CommandListRH InList) = 0;

		virtual ImageRH ImageCreate(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) = 0;
		virtual void ImageDestroy(ImageRH InImage) = 0;

		virtual ImageViewRH ImageViewCreate(ImageRH InImage) = 0;
		virtual void ImageViewDestroy(ImageViewRH InImageView) = 0;

	};

}
