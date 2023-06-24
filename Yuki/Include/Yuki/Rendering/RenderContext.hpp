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

		virtual Queue GetGraphicsQueue() const = 0;
		virtual Queue GetTransferQueue() const = 0;

		virtual DynamicArray<Swapchain> GetSwapchains() const = 0;

	public:
		virtual void QueueWaitIdle(Queue InQueue) = 0;
		virtual void QueueSubmitCommandLists(Queue InQueue, const InitializerList<CommandList>& InCommandLists, const InitializerList<Fence> InWaits, const InitializerList<Fence> InSignals) = 0;
		virtual void QueueAcquireImages(Queue InQueue, std::span<Swapchain> InSwapchains, const InitializerList<Fence>& InFences) = 0;
		virtual void QueuePresent(Queue InQueue, std::span<Swapchain> InSwapchains, const InitializerList<Fence>& InFences) = 0;

		virtual Swapchain CreateSwapchain(GenericWindow* InWindow) = 0;
		virtual void Destroy(Swapchain InSwapchain) = 0;

		virtual Fence CreateFence() = 0;
		virtual void DestroyFence(Fence InFence) = 0;
		virtual void FenceWait(Fence InFence, uint64_t InValue = 0) = 0;

		virtual CommandPool CreateCommandPool(Queue InQueue) = 0;
		virtual void CommandPoolReset(CommandPool InCommandPool) = 0;
		virtual void Destroy(CommandPool InCommandPool) = 0;

		virtual CommandList CreateCommandList(CommandPool InCommandPool) = 0;
		virtual void CommandListBegin(CommandList InCommandList) = 0;
		virtual void CommandListEnd(CommandList InCommandList) = 0;
		virtual void CommandListBeginRendering(CommandList InCommandList, Swapchain InSwapchain) = 0;
		virtual void CommandListEndRendering(CommandList InCommandList) = 0;
		virtual void CommandListBindPipeline(CommandList InCommandList, Pipeline InPipeline) = 0;
		virtual void CommandListBindBuffer(CommandList InCommandList, Buffer InBuffer) = 0;
		virtual void CommandListBindDescriptorSet(CommandList InCommandList, Pipeline InPipeline, DescriptorSet InSet) = 0;
		virtual void CommandListPushConstants(CommandList InCommandList, Pipeline InPipeline, const void* InData, uint32_t InDataSize, uint32_t InOffset = 0) = 0;
		virtual void CommandListTransitionImage(CommandList InCommandList, Image InImage, ImageLayout InNewLayout) = 0;
		virtual void CommandListCopyToBuffer(CommandList InCommandList, Buffer InDstBuffer, uint32_t InDstOffset, Buffer InSrcBuffer, uint32_t InSrcOffset, uint32_t InSize) = 0;
		virtual void CommandListCopyToImage(CommandList InCommandList, Image InDstImage, Buffer InSrcBuffer, uint32_t InSrcOffset) = 0;
		virtual void CommandListBlitImage(CommandList InCommandList, Image InDstImage, Image InSrcImage) = 0;
		virtual void CommandListDraw(CommandList InCommandList, uint32_t InVertexCount) = 0;
		virtual void CommandListDrawIndexed(CommandList InCommandList, uint32_t InIndexCount) = 0;

		virtual Image CreateImage(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) = 0;
		virtual void Destroy(Image InImage) = 0;
		virtual ImageView CreateImageView(Image InImage) = 0;
		virtual void Destroy(ImageView InImageView) = 0;

		virtual Sampler CreateSampler() = 0;
		virtual void Destroy(Sampler InSampler) = 0;

		virtual Shader CreateShader(const std::filesystem::path& InFilePath) = 0;
		virtual void Destroy(Shader InShader) = 0;

		virtual Pipeline CreatePipeline(const PipelineInfo& InPipelineInfo) = 0;
		virtual void Destroy(Pipeline InPipeline) = 0;

		virtual Buffer CreateBuffer(const BufferInfo& InBufferInfo) = 0;
		virtual void Destroy(Buffer InBuffer) = 0;
		virtual void BufferSetData(Buffer InBuffer, const void* InData, uint32_t InDataSize, uint32_t InBufferOffset = 0) = 0;
		virtual uint64_t BufferGetDeviceAddress(Buffer InBuffer) const = 0;

		virtual DescriptorSetLayout CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& InLayoutInfo) = 0;
		virtual void Destroy(DescriptorSetLayout InLayout) = 0;
		virtual DescriptorPool CreateDescriptorPool(std::span<DescriptorCount> InDescriptorCounts) = 0;
		virtual void Destroy(DescriptorPool InPool) = 0;
		virtual DescriptorSet DescriptorPoolAllocateDescriptorSet(DescriptorPool InPool, DescriptorSetLayout InLayout) = 0;
		virtual void DescriptorSetWrite(DescriptorSet InSet, uint32_t InBinding, std::span<Image> InImages, Sampler InSampler) = 0;
		virtual void DescriptorSetWrite(DescriptorSet InSet, uint32_t InBinding, std::span<Image> InImages, std::span<Sampler> InSamplers) = 0;
		virtual void DescriptorSetWrite(DescriptorSet InSet, uint32_t InBinding, std::span<ImageView> InImageViews, Sampler InSampler) = 0;
		virtual void DescriptorSetWrite(DescriptorSet InSet, uint32_t InBinding, std::span<ImageView> InImageViews, std::span<Sampler> InSamplers) = 0;
		virtual void DescriptorSetWrite(DescriptorSet InSet, uint32_t InBinding, std::span<std::pair<uint32_t, Buffer>> InBuffers) = 0;

	public:
		static Unique<RenderContext> New(RenderAPI InAPI);
	};

}
