#pragma once

#include "RenderAPI.hpp"
#include "RHI.hpp"
#include "PipelineBuilder.hpp"

#include "Yuki/Core/InitializerList.hpp"
#include "Yuki/Memory/Unique.hpp"

#include <vector>
#include <utility>
#include <type_traits>
#include <span>
#include <filesystem>

namespace Yuki {

	template<typename TKey, typename TElement>
	struct Registry
	{
		enum class ElementFlag : uint8_t
		{
			Empty = 0,
			Exists = 1,
		};

		std::vector<ElementFlag> Flags;
		std::vector<TKey> FreeList;
		std::vector<TElement> Elements;

		Registry()
		{
			Flags.push_back(ElementFlag::Empty);
			Elements.push_back({});
		}

		std::pair<TKey, TElement&> Acquire()
		{
			if (FreeList.empty())
			{
				auto& element = Elements.emplace_back();
				Flags.emplace_back(ElementFlag::Exists);
				return { TKey(Elements.size() - 1), element };
			}

			TKey key = FreeList.back();
			FreeList.pop_back();
			Flags[static_cast<std::underlying_type_t<TKey>>(key)] = ElementFlag::Exists;
			return { key, Get(key) };
		}

		void Return(TKey InKey)
		{
			Flags[static_cast<std::underlying_type_t<TKey>>(InKey)] = ElementFlag::Empty;
			FreeList.emplace_back(InKey);
		}

		bool IsValid(TKey InKey)
		{
			return Flags[static_cast<std::underlying_type_t<TKey>>(InKey)] == ElementFlag::Exists;
		}

		TElement& Get(TKey InKey) { return Elements[static_cast<std::underlying_type_t<TKey>>(InKey)]; }

		template<typename TFunction>
		void ForEach(TFunction&& InFunction)
		{
			for (size_t i = 0; i < Elements.size(); i++)
			{
				if (Flags[i] == ElementFlag::Exists)
					InFunction(TKey(i), Elements[i]);
			}
		}

		uint32_t GetCount() const { return uint32_t(Elements.size() - FreeList.size()); }
	};

	class GenericWindow;

	class RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void DeviceWaitIdle() const = 0;

		virtual Queue GetGraphicsQueue() const = 0;

	public:
		virtual void QueueSubmitCommandLists(const InitializerList<CommandList>& InCommandLists, const InitializerList<Fence> InWaits, const InitializerList<Fence> InSignals) = 0;
		virtual void QueueAcquireImages(std::span<Swapchain> InSwapchains, const InitializerList<Fence>& InFences) = 0;
		virtual void QueuePresent(std::span<Swapchain> InSwapchains, const InitializerList<Fence>& InFences) = 0;

		virtual Swapchain CreateSwapchain(GenericWindow* InWindow) = 0;
		virtual void Destroy(Swapchain InSwapchain) = 0;

		virtual Fence CreateFence() = 0;
		virtual void DestroyFence(Fence InFence) = 0;
		virtual void FenceWait(Fence InFence, uint64_t InValue = 0) = 0;

		virtual CommandPool CreateCommandPool() = 0;
		virtual void CommandPoolReset(CommandPool InCommandPool) = 0;
		virtual void Destroy(CommandPool InCommandPool) = 0;

		virtual CommandList CreateCommandList(CommandPool InCommandPool) = 0;
		virtual void CommandListBegin(CommandList InCommandList) = 0;
		virtual void CommandListEnd(CommandList InCommandList) = 0;
		virtual void CommandListBeginRendering(CommandList InCommandList, Swapchain InSwapchain) = 0;
		virtual void CommandListEndRendering(CommandList InCommandList) = 0;
		virtual void CommandListBindPipeline(CommandList InCommandList, Pipeline InPipeline) = 0;
		virtual void CommandListBindBuffer(CommandList InCommandList, Buffer InBuffer) = 0;
		virtual void CommandListTransitionImage(CommandList InCommandList, Image InImage, ImageLayout InNewLayout) = 0;
		virtual void CommandListCopyToBuffer(CommandList InCommandList, Buffer InDstBuffer, uint32_t InDstOffset, Buffer InSrcBuffer, uint32_t InSrcOffset, uint32_t InSize) = 0;
		virtual void CommandListCopyToImage(CommandList InCommandList, Image InDstImage, Buffer InSrcBuffer, uint32_t InSrcOffset) = 0;
		virtual void CommandListBlitImage(CommandList InCommandList, Image InDstImage, Image InSrcImage) = 0;
		virtual void CommandListDraw(CommandList InCommandList, uint32_t InVertexCount) = 0;

		virtual Image CreateImage(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) = 0;
		virtual void Destroy(Image InImage) = 0;
		virtual ImageView CreateImageView(Image InImage) = 0;
		virtual void Destroy(ImageView InImageView) = 0;

		virtual Shader CreateShader(const std::filesystem::path& InFilePath) = 0;
		virtual void Destroy(Shader InShader) = 0;

		virtual Pipeline CreatePipeline(const PipelineInfo& InPipelineInfo) = 0;
		virtual void Destroy(Pipeline InPipeline) = 0;

		virtual Buffer CreateBuffer(const BufferInfo& InBufferInfo) = 0;
		virtual void Destroy(Buffer InBuffer) = 0;
		virtual void BufferSetData(Buffer InBuffer, const void* InData, uint32_t InDataSize) = 0;

	public:
		static Unique<RenderContext> New(RenderAPI InAPI);
	};

}
