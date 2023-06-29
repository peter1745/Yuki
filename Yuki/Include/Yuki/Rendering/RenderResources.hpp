#pragma once

#include "RenderContext.hpp"

namespace Yuki {

	struct Queue
	{
		QueueHandle Handle{};
		RenderContext* Context{};

		void WaitIdle() { Context->QueueWaitIdle(Handle); }
		void SubmitCommandLists(const InitializerList<CommandListHandle>& InCommandLists, const InitializerList<FenceHandle> InWaits, const InitializerList<FenceHandle> InSignals)
		{
			Context->QueueSubmitCommandLists(Handle, InCommandLists, InWaits, InSignals);
		}

		void AcquireImages(std::span<SwapchainHandle> InSwapchains, const InitializerList<FenceHandle>& InFences)
		{
			Context->QueueAcquireImages(Handle, InSwapchains, InFences);
		}

		void Present(std::span<SwapchainHandle> InSwapchains, const InitializerList<FenceHandle>& InFences)
		{
			Context->QueuePresent(Handle, InSwapchains, InFences);
		}

		operator QueueHandle() const { return Handle; }
	};

	struct Swapchain
	{
		SwapchainHandle Handle{};
		RenderContext* Context{};
	
		operator SwapchainHandle() const { return Handle; }
	};

	struct Fence
	{
		FenceHandle Handle{};
		RenderContext* Context{};

		Fence() = default;
		Fence(RenderContext* InContext)
		{
			Context = InContext;
			Handle = InContext->CreateFence();
		}

		void Wait(uint64_t InValue = 0) { Context->FenceWait(Handle, InValue); }

		operator FenceHandle() const { return Handle; }
	};

	struct CommandList
	{
		CommandListHandle Handle{};
		RenderContext* Context{};

		void Begin() { Context->CommandListBegin(Handle); }
		void End() { Context->CommandListEnd(Handle); }

		void BeginRendering(SwapchainHandle InSwapchain) { Context->CommandListBeginRendering(Handle, InSwapchain); }
		void BeginRendering(const RenderTargetInfo& InRenderTarget) { Context->CommandListBeginRendering(Handle, InRenderTarget); }
		void EndRendering() { Context->CommandListEndRendering(Handle); }

		void BindPipeline(PipelineHandle InPipeline)
		{
			Context->CommandListBindPipeline(Handle, InPipeline);
		}

		void BindIndexBuffer(BufferHandle InBuffer, uint32_t InOffset, bool InUse32Bit = true)
		{
			Context->CommandListBindIndexBuffer(Handle, InBuffer, InOffset, InUse32Bit);
		}

		void BindDescriptorSet(PipelineHandle InPipeline, DescriptorSetHandle InSet)
		{
			Context->CommandListBindDescriptorSet(Handle, InPipeline, InSet);
		}

		void SetScissor(Scissor InScissor)
		{
			Context->CommandListSetScissor(Handle, InScissor);
		}

		void PushConstants(PipelineHandle InPipeline, const void* InData, uint32_t InDataSize, uint32_t InOffset = 0)
		{
			Context->CommandListPushConstants(Handle, InPipeline, InData, InDataSize, InOffset);
		}

		void TransitionImage(ImageHandle InImage, ImageLayout InNewLayout)
		{
			Context->CommandListTransitionImage(Handle, InImage, InNewLayout);
		}

		void CopyToBuffer(BufferHandle InDstBuffer, uint32_t InDstOffset, BufferHandle InSrcBuffer, uint32_t InSrcOffset, uint32_t InSize)
		{
			Context->CommandListCopyToBuffer(Handle, InDstBuffer, InDstOffset, InSrcBuffer, InSrcOffset, InSize);
		}

		void CopyToImage(ImageHandle InDstImage, BufferHandle InSrcBuffer, uint32_t InSrcOffset)
		{
			Context->CommandListCopyToImage(Handle, InDstImage, InSrcBuffer, InSrcOffset);
		}

		void BlitImage(ImageHandle InDstImage, ImageHandle InSrcImage)
		{
			Context->CommandListBlitImage(Handle, InDstImage, InSrcImage);
		}

		void Draw(uint32_t InVertexCount)
		{
			Context->CommandListDraw(Handle, InVertexCount);
		}

		void DrawIndexed(uint32_t InIndexCount, uint32_t InIndexOffset = 0)
		{
			Context->CommandListDrawIndexed(Handle, InIndexCount, InIndexOffset);
		}

		void PrepareSwapchainPresent(SwapchainHandle InSwapchain)
		{
			Context->CommandListPrepareSwapchainPresent(Handle, InSwapchain);
		}

		operator CommandListHandle() const { return Handle; }

	};

	struct CommandPool
	{
		CommandPoolHandle Handle{};
		RenderContext* Context{};

		CommandPool() = default;
		CommandPool(RenderContext* InContext, QueueHandle InQueue)
		{
			Context = InContext;
			Handle = InContext->CreateCommandPool(InQueue);
		}

		CommandList CreateCommandList()
		{
			return { Context->CreateCommandList(Handle), Context };
		}

		void Reset()
		{
			Context->CommandPoolReset(Handle);
		}

		operator CommandPoolHandle() const { return Handle; }
	};

	struct Image
	{
		ImageHandle Handle{};
		RenderContext* Context{};

		Image() = default;
		Image(RenderContext* InContext, uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage)
		{
			Context = InContext;
			Handle = InContext->CreateImage(InWidth, InHeight, InFormat, InUsage);
		}

		operator ImageHandle() const { return Handle; }
	};

	struct ImageView
	{
		ImageViewHandle Handle{};
		RenderContext* Context{};

		ImageView() = default;
		ImageView(RenderContext* InContext, ImageHandle InImageHandle)
		{
			Context = InContext;
			Handle = InContext->CreateImageView(InImageHandle);
		}

		operator ImageViewHandle() const { return Handle; }
	};

	struct Sampler
	{
		SamplerHandle Handle{};
		RenderContext* Context{};

		Sampler() = default;
		Sampler(RenderContext* InContext)
		{
			Context = InContext;
			Handle = InContext->CreateSampler();
		}

		operator SamplerHandle() const { return Handle; }
	};

	struct Shader
	{
		ShaderHandle Handle{};
		RenderContext* Context{};

		Shader() = default;
		Shader(RenderContext* InContext, const std::filesystem::path& InFilePath)
		{
			Context = InContext;
			Handle = InContext->CreateShader(InFilePath);
		}
		Shader(RenderContext* InContext, std::string_view InSource)
		{
			Context = InContext;
			Handle = InContext->CreateShader(InSource);
		}

		operator ShaderHandle() const { return Handle; }
	};

	struct Pipeline
	{
		PipelineHandle Handle{};
		RenderContext* Context{};

		operator PipelineHandle() const { return Handle; }
	};

	struct Buffer
	{
		BufferHandle Handle{};
		RenderContext* Context{};

		Buffer() = default;
		Buffer(RenderContext* InContext, const BufferInfo& InInfo)
		{
			Context = InContext;
			Handle = InContext->CreateBuffer(InInfo);
		}

		void SetData(const void* InData, uint32_t InDataSize, uint32_t InBufferOffset = 0)
		{
			Context->BufferSetData(Handle, InData, InDataSize, InBufferOffset);
		}

		uint64_t GetDeviceAddress() const { return Context->BufferGetDeviceAddress(Handle); }
		void* GetMappedData() { return Context->BufferGetMappedMemory(Handle); }

		operator BufferHandle() const { return Handle; }
	};

	struct DescriptorSetLayout
	{
		DescriptorSetLayoutHandle Handle{};
		RenderContext* Context{};

		operator DescriptorSetLayoutHandle() const { return Handle; }
	};

	struct DescriptorSet
	{
		DescriptorSetHandle Handle{};
		RenderContext* Context{};

		void Write(uint32_t InBinding, std::span<ImageHandle> InImages, SamplerHandle InSampler, uint32_t InArrayOffset = 0)
		{
			Context->DescriptorSetWrite(Handle, InBinding, InImages, InSampler, InArrayOffset);
		}

		void Write(uint32_t InBinding, std::span<ImageHandle> InImages, std::span<SamplerHandle> InSamplers, uint32_t InArrayOffset = 0)
		{
			Context->DescriptorSetWrite(Handle, InBinding, InImages, InSamplers, InArrayOffset);
		}

		void Write(uint32_t InBinding, std::span<ImageViewHandle> InImageViews, SamplerHandle InSampler, uint32_t InArrayOffset = 0)
		{
			Context->DescriptorSetWrite(Handle, InBinding, InImageViews, InSampler, InArrayOffset);
		}

		void Write(uint32_t InBinding, std::span<ImageViewHandle> InImageViews, std::span<SamplerHandle> InSamplers, uint32_t InArrayOffset = 0)
		{
			Context->DescriptorSetWrite(Handle, InBinding, InImageViews, InSamplers, InArrayOffset);
		}

		void Write(uint32_t InBinding, std::span<std::pair<uint32_t, BufferHandle>> InBuffers, uint32_t InArrayOffset = 0)
		{
			Context->DescriptorSetWrite(Handle, InBinding, InBuffers, InArrayOffset);
		}

		operator DescriptorSetHandle() const { return Handle; }

	};
	
	struct DescriptorPool
	{
		DescriptorPoolHandle Handle{};
		RenderContext* Context{};

		DescriptorPool() = default;
		DescriptorPool(RenderContext* InContext, std::span<DescriptorCount> InDescriptorCounts)
		{
			Context = InContext;
			Handle = InContext->CreateDescriptorPool(InDescriptorCounts);
		}

		DescriptorSet AllocateDescriptorSet(DescriptorSetLayoutHandle InLayout)
		{
			return { Context->DescriptorPoolAllocateDescriptorSet(Handle, InLayout), Context };
		}

		operator DescriptorPoolHandle() const { return Handle; }

	};

}