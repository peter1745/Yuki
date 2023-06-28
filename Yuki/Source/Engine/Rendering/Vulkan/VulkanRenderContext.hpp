#pragma once

#include "Core/ResourceRegistry.hpp"
#include "Rendering/RenderContext.hpp"

#include "VulkanQueue.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanFence.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanImage.hpp"
#include "VulkanSampler.hpp"
#include "VulkanShader.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanDescriptorPool.hpp"

#include <vma/vk_mem_alloc.h>

namespace Yuki {

	class VulkanRenderContext final : public RenderContext
	{
	public:
		~VulkanRenderContext();

		void DeviceWaitIdle() const override;

		QueueHandle GetGraphicsQueue(size_t InIndex = 0) const override
		{
			//std::scoped_lock lock(m_Mutex);
			return m_GraphicsQueues[InIndex];
		}

		QueueHandle GetTransferQueue(size_t InIndex = 0) const override
		{
			//std::scoped_lock lock(m_Mutex1);
			return m_TransferQueues[InIndex];
		}

		DynamicArray<SwapchainHandle> GetSwapchains() const override;

	public:
		void QueueWaitIdle(QueueHandle InQueue) override;
		void QueueSubmitCommandLists(QueueHandle InQueue, const InitializerList<CommandListHandle>& InCommandLists, const InitializerList<FenceHandle> InWaits, const InitializerList<FenceHandle> InSignals) override;
		void QueueAcquireImages(QueueHandle InQueue, std::span<SwapchainHandle> InSwapchains, const InitializerList<FenceHandle>& InFences) override;
		void QueuePresent(QueueHandle InQueue, std::span<SwapchainHandle> InSwapchains, const InitializerList<FenceHandle>& InFences) override;

	public:
		SwapchainHandle CreateSwapchain(GenericWindow* InWindow) override;
		void Destroy(SwapchainHandle InSwapchain) override;

		FenceHandle CreateFence() override;
		void Destroy(FenceHandle InFence) override;
		void FenceWait(FenceHandle InFence, uint64_t InValue = 0) override;

		CommandPoolHandle CreateCommandPool(QueueHandle InQueue) override;
		void CommandPoolReset(CommandPoolHandle InCommandPool) override;
		void Destroy(CommandPoolHandle InCommandPool) override;

		CommandListHandle CreateCommandList(CommandPoolHandle InCommandPool) override;
		void CommandListBegin(CommandListHandle InCommandList) override;
		void CommandListEnd(CommandListHandle InCommandList) override;
		void CommandListBeginRendering(CommandListHandle InCommandList, SwapchainHandle InSwapchain) override;
		void CommandListEndRendering(CommandListHandle InCommandList) override;
		void CommandListBindPipeline(CommandListHandle InCommandList, PipelineHandle InPipeline) override;
		void CommandListBindBuffer(CommandListHandle InCommandList, BufferHandle InBuffer) override;
		void CommandListBindIndexBuffer(CommandListHandle InCommandList, BufferHandle InBuffer, uint32_t InOffset, bool InUse32Bit = true) override;
		void CommandListBindDescriptorSet(CommandListHandle InCommandList, PipelineHandle InPipeline, DescriptorSetHandle InSet) override;
		void CommandListSetScissor(CommandListHandle InCommandList, Scissor InScissor) override;
		void CommandListPushConstants(CommandListHandle InCommandList, PipelineHandle InPipeline, const void* InData, uint32_t InDataSize, uint32_t InOffset = 0) override;
		void CommandListTransitionImage(CommandListHandle InCommandList, ImageHandle InImage, ImageLayout InNewLayout) override;
		void CommandListCopyToBuffer(CommandListHandle InCommandList, BufferHandle InDstBuffer, uint32_t InDstOffset, BufferHandle InSrcBuffer, uint32_t InSrcOffset, uint32_t InSize) override;
		void CommandListCopyToImage(CommandListHandle InCommandList, ImageHandle InDstImage, BufferHandle InSrcBuffer, uint32_t InSrcOffset) override;
		void CommandListBlitImage(CommandListHandle InCommandList, ImageHandle InDstImage, ImageHandle InSrcImage) override;
		void CommandListDraw(CommandListHandle InCommandList, uint32_t InVertexCount) override;
		void CommandListDrawIndexed(CommandListHandle InCommandList, uint32_t InIndexCount, uint32_t InIndexOffset = 0) override;
		void CommandListPrepareSwapchainPresent(CommandListHandle InCommandList, SwapchainHandle InSwapchain) override;

		ImageHandle CreateImage(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) override;
		void Destroy(ImageHandle InImage) override;
		ImageViewHandle CreateImageView(ImageHandle InImage) override;
		void Destroy(ImageViewHandle InImageView) override;

		SamplerHandle CreateSampler() override;
		void Destroy(SamplerHandle InSampler) override;

		ShaderHandle CreateShader(const std::filesystem::path& InFilePath) override;
		ShaderHandle CreateShader(std::string_view InSource) override;
		void Destroy(ShaderHandle InShader) override;

		PipelineHandle CreatePipeline(const PipelineInfo& InPipelineInfo) override;
		void Destroy(PipelineHandle InPipeline) override;

		BufferHandle CreateBuffer(const BufferInfo& InBufferInfo) override;
		void Destroy(BufferHandle InBuffer) override;
		void BufferSetData(BufferHandle InBuffer, const void* InData, uint32_t InDataSize, uint32_t InBufferOffset = 0) override;
		uint64_t BufferGetDeviceAddress(BufferHandle InBuffer) const override;
		void* BufferGetMappedMemory(BufferHandle InBuffer) override;

		DescriptorSetLayoutHandle CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& InLayoutInfo) override;
		void Destroy(DescriptorSetLayoutHandle InLayout) override;
		DescriptorPoolHandle CreateDescriptorPool(std::span<DescriptorCount> InDescriptorCounts) override;
		void Destroy(DescriptorPoolHandle InPool) override;
		DescriptorSetHandle DescriptorPoolAllocateDescriptorSet(DescriptorPoolHandle InPool, DescriptorSetLayoutHandle InLayout) override;
		void DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageHandle> InImages, SamplerHandle InSampler, uint32_t InArrayOffset) override;
		void DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageHandle> InImages, std::span<SamplerHandle> InSamplers, uint32_t InArrayOffset) override;
		void DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageViewHandle> InImageViews, SamplerHandle InSampler, uint32_t InArrayOffset) override;
		void DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageViewHandle> InImageViews, std::span<SamplerHandle> InSamplers, uint32_t InArrayOffset) override;
		void DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<std::pair<uint32_t, BufferHandle>> InBuffers, uint32_t InArrayOffset) override;

	private:
		void RecreateSwapchain(VulkanSwapchain& InSwapchain);

	private:
		VulkanRenderContext();

		void SelectSuitablePhysicalDevice();
		void CreateLogicalDevice(const DynamicArray<const char*>& InDeviceLayers);
		void SetupDebugUtilsMessenger();

		uint32_t SelectQueue(VkQueueFlags InQueueFlags) const;

	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;

		VkDebugUtilsMessengerEXT m_DebugUtilsMessengerHandle = VK_NULL_HANDLE;

		mutable std::shared_mutex m_Mutex;
		mutable std::shared_mutex m_Mutex1;
		DynamicArray<QueueHandle> m_GraphicsQueues;
		DynamicArray<QueueHandle> m_TransferQueues;
		DynamicArray<QueueHandle> m_DeviceQueues;

		VmaAllocator m_Allocator{};

	private:
		ResourceRegistry<QueueHandle, VulkanQueue> m_Queues;
		ResourceRegistry<SwapchainHandle, VulkanSwapchain> m_Swapchains;
		ResourceRegistry<FenceHandle, VulkanFence> m_Fences;
		ResourceRegistry<CommandPoolHandle, VulkanCommandPool> m_CommandPools;
		ResourceRegistry<CommandListHandle, VulkanCommandList> m_CommandLists;
		ResourceRegistry<ImageHandle, VulkanImage> m_Images;
		ResourceRegistry<ImageViewHandle, VulkanImageView> m_ImageViews;
		ResourceRegistry<SamplerHandle, VulkanSampler> m_Samplers;
		ResourceRegistry<ShaderHandle, VulkanShader> m_Shaders;
		ResourceRegistry<PipelineHandle, VulkanPipeline> m_Pipelines;
		ResourceRegistry<BufferHandle, VulkanBuffer> m_Buffers;
		ResourceRegistry<DescriptorSetLayoutHandle, VulkanDescriptorSetLayout> m_DescriptorSetLayouts;
		ResourceRegistry<DescriptorPoolHandle, VulkanDescriptorPool> m_DescriptorPools;
		ResourceRegistry<DescriptorSetHandle, VulkanDescriptorSet> m_DescriptorSets;

	private:
		friend class RenderContext;
	};

}
