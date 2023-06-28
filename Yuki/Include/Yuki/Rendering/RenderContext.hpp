#pragma once

#include "RenderAPI.hpp"
#include "RHI.hpp"
#include "PipelineBuilder.hpp"

#include "Yuki/Core/InitializerList.hpp"
#include "Yuki/Memory/Unique.hpp"

#include <span>
#include <filesystem>

namespace Yuki {

	class GenericWindow;

	class RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void DeviceWaitIdle() const = 0;

		virtual QueueHandle GetGraphicsQueue(size_t InIndex = 0) const = 0;
		virtual QueueHandle GetTransferQueue(size_t InIndex = 0) const = 0;

		virtual DynamicArray<SwapchainHandle> GetSwapchains() const = 0;

	public:
		virtual void QueueWaitIdle(QueueHandle InQueue) = 0;
		virtual void QueueSubmitCommandLists(QueueHandle InQueue, const InitializerList<CommandListHandle>& InCommandLists, const InitializerList<FenceHandle> InWaits, const InitializerList<FenceHandle> InSignals) = 0;
		virtual void QueueAcquireImages(QueueHandle InQueue, std::span<SwapchainHandle> InSwapchains, const InitializerList<FenceHandle>& InFences) = 0;
		virtual void QueuePresent(QueueHandle InQueue, std::span<SwapchainHandle> InSwapchains, const InitializerList<FenceHandle>& InFences) = 0;

		virtual SwapchainHandle CreateSwapchain(GenericWindow* InWindow) = 0;
		virtual void Destroy(SwapchainHandle InSwapchain) = 0;

		virtual FenceHandle CreateFence() = 0;
		virtual void Destroy(FenceHandle InFence) = 0;
		virtual void FenceWait(FenceHandle InFence, uint64_t InValue = 0) = 0;

		virtual CommandPoolHandle CreateCommandPool(QueueHandle InQueue) = 0;
		virtual void CommandPoolReset(CommandPoolHandle InCommandPool) = 0;
		virtual void Destroy(CommandPoolHandle InCommandPool) = 0;

		virtual CommandListHandle CreateCommandList(CommandPoolHandle InCommandPool) = 0;
		virtual void CommandListBegin(CommandListHandle InCommandList) = 0;
		virtual void CommandListEnd(CommandListHandle InCommandList) = 0;
		virtual void CommandListBeginRendering(CommandListHandle InCommandList, SwapchainHandle InSwapchain) = 0;
		virtual void CommandListEndRendering(CommandListHandle InCommandList) = 0;
		virtual void CommandListBindPipeline(CommandListHandle InCommandList, PipelineHandle InPipeline) = 0;
		virtual void CommandListBindBuffer(CommandListHandle InCommandList, BufferHandle InBuffer) = 0;
		virtual void CommandListBindIndexBuffer(CommandListHandle InCommandList, BufferHandle InBuffer, uint32_t InOffset, bool InUse32Bit = true) = 0;
		virtual void CommandListBindDescriptorSet(CommandListHandle InCommandList, PipelineHandle InPipeline, DescriptorSetHandle InSet) = 0;
		virtual void CommandListSetScissor(CommandListHandle InCommandList, Scissor InScissor) = 0;
		virtual void CommandListPushConstants(CommandListHandle InCommandList, PipelineHandle InPipeline, const void* InData, uint32_t InDataSize, uint32_t InOffset = 0) = 0;
		virtual void CommandListTransitionImage(CommandListHandle InCommandList, ImageHandle InImage, ImageLayout InNewLayout) = 0;
		virtual void CommandListCopyToBuffer(CommandListHandle InCommandList, BufferHandle InDstBuffer, uint32_t InDstOffset, BufferHandle InSrcBuffer, uint32_t InSrcOffset, uint32_t InSize) = 0;
		virtual void CommandListCopyToImage(CommandListHandle InCommandList, ImageHandle InDstImage, BufferHandle InSrcBuffer, uint32_t InSrcOffset) = 0;
		virtual void CommandListBlitImage(CommandListHandle InCommandList, ImageHandle InDstImage, ImageHandle InSrcImage) = 0;
		virtual void CommandListDraw(CommandListHandle InCommandList, uint32_t InVertexCount) = 0;
		virtual void CommandListDrawIndexed(CommandListHandle InCommandList, uint32_t InIndexCount, uint32_t InIndexOffset = 0) = 0;
		virtual void CommandListPrepareSwapchainPresent(CommandListHandle InCommandList, SwapchainHandle InSwapchain) = 0;

		virtual ImageHandle CreateImage(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) = 0;
		virtual void Destroy(ImageHandle InImage) = 0;
		virtual ImageViewHandle CreateImageView(ImageHandle InImage) = 0;
		virtual void Destroy(ImageViewHandle InImageView) = 0;

		virtual SamplerHandle CreateSampler() = 0;
		virtual void Destroy(SamplerHandle InSampler) = 0;

		virtual ShaderHandle CreateShader(const std::filesystem::path& InFilePath) = 0;
		virtual ShaderHandle CreateShader(std::string_view InSource) = 0;
		virtual void Destroy(ShaderHandle InShader) = 0;

		virtual PipelineHandle CreatePipeline(const PipelineInfo& InPipelineInfo) = 0;
		virtual void Destroy(PipelineHandle InPipeline) = 0;

		virtual BufferHandle CreateBuffer(const BufferInfo& InBufferInfo) = 0;
		virtual void Destroy(BufferHandle InBuffer) = 0;
		virtual void BufferSetData(BufferHandle InBuffer, const void* InData, uint32_t InDataSize, uint32_t InBufferOffset = 0) = 0;
		virtual uint64_t BufferGetDeviceAddress(BufferHandle InBuffer) const = 0;
		virtual void* BufferGetMappedMemory(BufferHandle InBuffer) = 0;

		virtual DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& InLayoutInfo) = 0;
		virtual void Destroy(DescriptorSetLayoutHandle InLayout) = 0;
		virtual DescriptorPoolHandle CreateDescriptorPool(std::span<DescriptorCount> InDescriptorCounts) = 0;
		virtual void Destroy(DescriptorPoolHandle InPool) = 0;
		virtual DescriptorSetHandle DescriptorPoolAllocateDescriptorSet(DescriptorPoolHandle InPool, DescriptorSetLayoutHandle InLayout) = 0;
		virtual void DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageHandle> InImages, SamplerHandle InSampler, uint32_t InArrayOffset = 0) = 0;
		virtual void DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageHandle> InImages, std::span<SamplerHandle> InSamplers, uint32_t InArrayOffset = 0) = 0;
		virtual void DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageViewHandle> InImageViews, SamplerHandle InSampler, uint32_t InArrayOffset = 0) = 0;
		virtual void DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageViewHandle> InImageViews, std::span<SamplerHandle> InSamplers, uint32_t InArrayOffset = 0) = 0;
		virtual void DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<std::pair<uint32_t, BufferHandle>> InBuffers, uint32_t InArrayOffset = 0) = 0;

	public:
		static Unique<RenderContext> New(RenderAPI InAPI);
	};

}
