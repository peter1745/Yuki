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

		Queue GetGraphicsQueue(size_t InQueueIndex) const override { return m_GraphicsQueues[InQueueIndex]; }
		Queue GetTransferQueue() const override { return m_TransferQueue; }

		DynamicArray<Swapchain> GetSwapchains() const override;

	public:
		void QueueWaitIdle(Queue InQueue) override;
		void QueueSubmitCommandLists(Queue InQueue, const InitializerList<CommandList>& InCommandLists, const InitializerList<Fence> InWaits, const InitializerList<Fence> InSignals) override;
		void QueueAcquireImages(Queue InQueue, std::span<Swapchain> InSwapchains, const InitializerList<Fence>& InFences) override;
		void QueuePresent(Queue InQueue, std::span<Swapchain> InSwapchains, const InitializerList<Fence>& InFences) override;

	public:
		Swapchain CreateSwapchain(GenericWindow* InWindow) override;
		void Destroy(Swapchain InSwapchain) override;

		Fence CreateFence() override;
		void DestroyFence(Fence InFence) override;
		void FenceWait(Fence InFence, uint64_t InValue = 0) override;

		CommandPool CreateCommandPool(Queue InQueue) override;
		void CommandPoolReset(CommandPool InCommandPool) override;
		void Destroy(CommandPool InCommandPool) override;

		CommandList CreateCommandList(CommandPool InCommandPool) override;
		void CommandListBegin(CommandList InCommandList) override;
		void CommandListEnd(CommandList InCommandList) override;
		void CommandListBeginRendering(CommandList InCommandList, Swapchain InSwapchain) override;
		void CommandListEndRendering(CommandList InCommandList) override;
		void CommandListBindPipeline(CommandList InCommandList, Pipeline InPipeline) override;
		void CommandListBindBuffer(CommandList InCommandList, Buffer InBuffer) override;
		void CommandListBindDescriptorSet(CommandList InCommandList, Pipeline InPipeline, DescriptorSet InSet) override;
		void CommandListPushConstants(CommandList InCommandList, Pipeline InPipeline, const void* InData, uint32_t InDataSize, uint32_t InOffset = 0) override;
		void CommandListTransitionImage(CommandList InCommandList, Image InImage, ImageLayout InNewLayout) override;
		void CommandListCopyToBuffer(CommandList InCommandList, Buffer InDstBuffer, uint32_t InDstOffset, Buffer InSrcBuffer, uint32_t InSrcOffset, uint32_t InSize) override;
		void CommandListCopyToImage(CommandList InCommandList, Image InDstImage, Buffer InSrcBuffer, uint32_t InSrcOffset) override;
		void CommandListBlitImage(CommandList InCommandList, Image InDstImage, Image InSrcImage) override;
		void CommandListDraw(CommandList InCommandList, uint32_t InVertexCount) override;
		void CommandListDrawIndexed(CommandList InCommandList, uint32_t InIndexCount) override;

		Image CreateImage(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) override;
		void Destroy(Image InImage) override;
		ImageView CreateImageView(Image InImage) override;
		void Destroy(ImageView InImageView) override;

		Sampler CreateSampler() override;
		void Destroy(Sampler InSampler) override;

		Shader CreateShader(const std::filesystem::path& InFilePath) override;
		void Destroy(Shader InShader) override;

		Pipeline CreatePipeline(const PipelineInfo& InPipelineInfo) override;
		void Destroy(Pipeline InPipeline) override;

		Buffer CreateBuffer(const BufferInfo& InBufferInfo) override;
		void Destroy(Buffer InBuffer) override;
		void BufferSetData(Buffer InBuffer, const void* InData, uint32_t InDataSize, uint32_t InBufferOffset = 0) override;
		uint64_t BufferGetDeviceAddress(Buffer InBuffer) const override;

		DescriptorSetLayout CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& InLayoutInfo) override;
		void Destroy(DescriptorSetLayout InLayout) override;
		DescriptorPool CreateDescriptorPool(std::span<DescriptorCount> InDescriptorCounts) override;
		void Destroy(DescriptorPool InPool) override;
		DescriptorSet DescriptorPoolAllocateDescriptorSet(DescriptorPool InPool, DescriptorSetLayout InLayout) override;
		void DescriptorSetWrite(DescriptorSet InSet, uint32_t InBinding, std::span<Image> InImages, Sampler InSampler, uint32_t InArrayOffset) override;
		void DescriptorSetWrite(DescriptorSet InSet, uint32_t InBinding, std::span<Image> InImages, std::span<Sampler> InSamplers, uint32_t InArrayOffset) override;
		void DescriptorSetWrite(DescriptorSet InSet, uint32_t InBinding, std::span<ImageView> InImageViews, Sampler InSampler, uint32_t InArrayOffset) override;
		void DescriptorSetWrite(DescriptorSet InSet, uint32_t InBinding, std::span<ImageView> InImageViews, std::span<Sampler> InSamplers, uint32_t InArrayOffset) override;
		void DescriptorSetWrite(DescriptorSet InSet, uint32_t InBinding, std::span<std::pair<uint32_t, Buffer>> InBuffers, uint32_t InArrayOffset) override;

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

		DynamicArray<Queue> m_GraphicsQueues;
		Queue m_TransferQueue;

		VmaAllocator m_Allocator{};

	private:
		ResourceRegistry<Queue, VulkanQueue> m_Queues;
		ResourceRegistry<Swapchain, VulkanSwapchain> m_Swapchains;
		ResourceRegistry<Fence, VulkanFence> m_Fences;
		ResourceRegistry<CommandPool, VulkanCommandPool> m_CommandPools;
		ResourceRegistry<CommandList, VulkanCommandList> m_CommandLists;
		ResourceRegistry<Image, VulkanImage> m_Images;
		ResourceRegistry<ImageView, VulkanImageView> m_ImageViews;
		ResourceRegistry<Sampler, VulkanSampler> m_Samplers;
		ResourceRegistry<Shader, VulkanShader> m_Shaders;
		ResourceRegistry<Pipeline, VulkanPipeline> m_Pipelines;
		ResourceRegistry<Buffer, VulkanBuffer> m_Buffers;
		ResourceRegistry<DescriptorSetLayout, VulkanDescriptorSetLayout> m_DescriptorSetLayouts;
		ResourceRegistry<DescriptorPool, VulkanDescriptorPool> m_DescriptorPools;
		ResourceRegistry<DescriptorSet, VulkanDescriptorSet> m_DescriptorSets;

	private:
		friend class RenderContext;
	};

}
