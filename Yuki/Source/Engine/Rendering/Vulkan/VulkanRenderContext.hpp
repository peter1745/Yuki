#pragma once

#include "Rendering/RenderContext.hpp"

#include "VulkanQueue.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanFence.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanImage.hpp"
#include "VulkanShader.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanBuffer.hpp"

#include <vma/vk_mem_alloc.h>

namespace Yuki {

	class VulkanRenderContext final : public RenderContext
	{
	public:
		~VulkanRenderContext() = default;

		void DeviceWaitIdle() const override;

		Queue GetGraphicsQueue() const override { return m_GraphicsQueue; }

	public:
		void QueueSubmitCommandLists(const InitializerList<CommandList>& InCommandLists, const InitializerList<Fence> InWaits, const InitializerList<Fence> InSignals) override;
		void QueueAcquireImages(std::span<Swapchain> InSwapchains, const InitializerList<Fence>& InFences) override;
		void QueuePresent(std::span<Swapchain> InSwapchains, const InitializerList<Fence>& InFences) override;

	public:
		Swapchain CreateSwapchain(GenericWindow* InWindow) override;
		void Destroy(Swapchain InSwapchain) override;

		Fence CreateFence() override;
		void DestroyFence(Fence InFence) override;
		void FenceWait(Fence InFence, uint64_t InValue = 0) override;

		CommandPool CreateCommandPool() override;
		void CommandPoolReset(CommandPool InCommandPool) override;
		void Destroy(CommandPool InCommandPool) override;

		CommandList CreateCommandList(CommandPool InCommandPool) override;
		void CommandListBegin(CommandList InCommandList) override;
		void CommandListEnd(CommandList InCommandList) override;
		void CommandListBeginRendering(CommandList InCommandList, Swapchain InSwapchain) override;
		void CommandListEndRendering(CommandList InCommandList) override;
		void CommandListBindPipeline(CommandList InCommandList, Pipeline InPipeline) override;
		void CommandListBindBuffer(CommandList InCommandList, Buffer InBuffer) override;
		void CommandListTransitionImage(CommandList InCommandList, Image InImage, ImageLayout InNewLayout) override;
		void CommandListCopyToBuffer(CommandList InCommandList, Buffer InDstBuffer, uint32_t InDstOffset, Buffer InSrcBuffer, uint32_t InSrcOffset, uint32_t InSize) override;
		void CommandListCopyToImage(CommandList InCommandList, Image InDstImage, Buffer InSrcBuffer, uint32_t InSrcOffset) override;
		void CommandListBlitImage(CommandList InCommandList, Image InDstImage, Image InSrcImage) override;
		void CommandListDraw(CommandList InCommandList, uint32_t InVertexCount) override;

		Image CreateImage(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) override;
		void Destroy(Image InImage) override;
		ImageView CreateImageView(Image InImage) override;
		void Destroy(ImageView InImageView) override;

		Shader CreateShader(const std::filesystem::path& InFilePath) override;
		void Destroy(Shader InShader) override;

		Pipeline CreatePipeline(const PipelineInfo& InPipelineInfo) override;
		void Destroy(Pipeline InPipeline) override;

		Buffer CreateBuffer(const BufferInfo& InBufferInfo) override;
		void Destroy(Buffer InBuffer) override;
		void BufferSetData(Buffer InBuffer, const void* InData, uint32_t InDataSize) override;

	private:
		void RecreateSwapchain(VulkanSwapchain& InSwapchain);

	private:
		VulkanRenderContext();

		void SelectSuitablePhysicalDevice();
		void CreateLogicalDevice(const DynamicArray<const char*>& InDeviceLayers);
		void SetupDebugUtilsMessenger();

	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;

		VkDebugUtilsMessengerEXT m_DebugUtilsMessengerHandle = VK_NULL_HANDLE;

		Queue m_GraphicsQueue;

		VmaAllocator m_Allocator{};

		// NOTE(Peter): Temporary. User should be responsible for this?
		CommandPool m_PresentTransitionPool = {};

	private:
		Registry<Queue, VulkanQueue> m_Queues;
		Registry<Swapchain, VulkanSwapchain> m_Swapchains;
		Registry<Fence, VulkanFence> m_Fences;
		Registry<CommandPool, VulkanCommandPool> m_CommandPools;
		Registry<CommandList, VulkanCommandList> m_CommandLists;
		Registry<Image, VulkanImage> m_Images;
		Registry<ImageView, VulkanImageView> m_ImageViews;
		Registry<Shader, VulkanShader> m_Shaders;
		Registry<Pipeline, VulkanPipeline> m_Pipelines;
		Registry<Buffer, VulkanBuffer> m_Buffers;

	private:
		friend class RenderContext;
	};

}
