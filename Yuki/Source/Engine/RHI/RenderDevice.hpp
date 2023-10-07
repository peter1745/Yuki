#pragma once

#include "RenderHandles.hpp"
#include "RenderFeatures.hpp"
#include "RenderTarget.hpp"
#include "Barriers.hpp"
#include "PipelineInfo.hpp"

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
		virtual void CommandListCopyBuffer(CommandListRH InList, BufferRH InDest, BufferRH InSrc) = 0;
		virtual void CommandListPushConstants(CommandListRH InList, PipelineRH InPipeline, ShaderStage InStages, const void* InData, uint32_t InDataSize) = 0;
		virtual void CommandListPushConstants(CommandListRH InList, RayTracingPipelineRH InPipeline, ShaderStage InStages, const void* InData, uint32_t InDataSize) = 0;
		virtual void CommandListBindDescriptorSets(CommandListRH InList, PipelineRH InPipeline, Span<DescriptorSetRH> InDescriptorSets) = 0;
		virtual void CommandListBindDescriptorSets(CommandListRH InList, RayTracingPipelineRH InPipeline, Span<DescriptorSetRH> InDescriptorSets) = 0;
		virtual void CommandListBindPipeline(CommandListRH InList, PipelineRH InPipeline) = 0;
		virtual void CommandListBindPipeline(CommandListRH InList, RayTracingPipelineRH InPipeline) = 0;
		virtual void CommandListBindIndexBuffer(CommandListRH InList, BufferRH InBuffer) = 0;
		struct Viewport
		{
			float X;
			float Y;
			float Width;
			float Height;
		};
		virtual void CommandListSetViewport(CommandListRH InList, Viewport InViewport) = 0;
		virtual void CommandListDrawIndexed(CommandListRH InList, uint32_t InIndexCount, uint32_t InIndexOffset, uint32_t InInstanceIndex) = 0;
		virtual void CommandListTraceRay(CommandListRH InList, RayTracingPipelineRH InPipeline, uint32_t InWidth, uint32_t InHeight) = 0;
		virtual void CommandListEnd(CommandListRH InList) = 0;

		virtual ImageRH ImageCreate(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) = 0;
		virtual void ImageDestroy(ImageRH InImage) = 0;

		virtual ImageViewRH ImageViewCreate(ImageRH InImage) = 0;
		virtual void ImageViewDestroy(ImageViewRH InImageView) = 0;

		virtual BufferRH BufferCreate(uint64_t InSize, BufferUsage InUsage, bool InHostAccess = false) = 0;
		virtual void BufferSetData(BufferRH InBuffer, const void* InData, uint64_t InDataSize = ~0) = 0;
		virtual uint64_t BufferGetDeviceAddress(BufferRH InBuffer) = 0;
		virtual void* BufferGetMappedMemory(BufferRH InBuffer) = 0;
		virtual void BufferDestroy(BufferRH InBuffer) = 0;

		virtual DescriptorSetLayoutRH DescriptorSetLayoutCreate(const DescriptorSetLayoutInfo& InLayoutInfo) = 0;
		virtual void DescriptorSetLayoutDestroy(DescriptorSetLayoutRH InLayout) = 0;
		virtual DescriptorPoolRH DescriptorPoolCreate(Span<DescriptorCount> InDescriptorCounts) = 0;
		virtual void DescriptorPoolDestroy(DescriptorPoolRH InPool) = 0;
		virtual DescriptorSetRH DescriptorPoolAllocateDescriptorSet(DescriptorPoolRH InPool, DescriptorSetLayoutRH InLayout) = 0;
		virtual void DescriptorSetWrite(DescriptorSetRH InSet, uint32_t InBinding, Span<ImageViewRH> InImageViews, uint32_t InArrayOffset) = 0;

		virtual PipelineRH PipelineCreate(const PipelineInfo& InPipelineInfo) = 0;
		virtual void PipelineDestroy(PipelineRH InPipeline) = 0;

		virtual RayTracingPipelineRH RayTracingPipelineCreate(const RayTracingPipelineInfo& InPipelineInfo) = 0;
		virtual void RayTracingPipelineDestroy(RayTracingPipelineRH InPipeline) = 0;

		virtual AccelerationStructureRH AccelerationStructureCreate(BufferRH InVertexBuffer, BufferRH InIndexBuffer) = 0;
		virtual uint64_t AccelerationStructureGetTopLevelAddress(AccelerationStructureRH InAccelerationStructure) = 0;
		virtual void AccelerationStructureDestroy(AccelerationStructureRH InAccelerationStructure) = 0;

	};

}
