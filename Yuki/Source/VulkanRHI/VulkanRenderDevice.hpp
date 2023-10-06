#pragma once

#include "Engine/Containers/ResourceRegistry.hpp"

#include "Engine/RHI/RenderDevice.hpp"
#include "Engine/RHI/RenderFeatures.hpp"

#include "Features/VulkanFeature.hpp"

#include "VulkanQueue.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanFence.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanImage.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanShaderCompiler.hpp"
#include "VulkanDescriptors.hpp"
#include "VulkanAccelerationStructure.hpp"

namespace Yuki::RHI {

	class VulkanRenderDevice final : public RenderDevice
	{
	public:
		bool IsFeatureEnabled(RendererFeature InFeature) const override;

		template<std::derived_from<VulkanFeature> TFeature>
		TFeature& GetFeature() const
		{
			return m_Features.at(TFeature::GetRendererFeature());
		}

	public:
		QueueRH QueueRequest(QueueType InType) override;
		void QueueAcquireImages(QueueRH InQueue, Span<SwapchainRH> InSwapchains, Span<FenceRH> InFences) override;
		void QueueSubmit(QueueRH InQueue, Span<CommandListRH> InCommandLists, Span<FenceRH> InWaits, Span<FenceRH> InSignals) override;
		void QueuePresent(QueueRH InQueue, Span<SwapchainRH> InSwapchains, Span<FenceRH> InFences) override;

		SwapchainRH SwapchainCreate(const WindowSystem& InWindowSystem, UniqueID InWindowHandle) override;
		ImageRH SwapchainGetCurrentImage(SwapchainRH InSwapchain) override;
		ImageViewRH SwapchainGetCurrentImageView(SwapchainRH InSwapchain) override;
		void SwapchainDestroy(SwapchainRH InSwapchain) override;
		void SwapchainRecreate(VulkanSwapchain& InSwapchain);

		FenceRH FenceCreate() override;
		void FenceWait(FenceRH InFence, uint64_t InValue = 0) override;
		void FenceDestroy(FenceRH InFence) override;

		CommandPoolRH CommandPoolCreate(QueueRH InQueue) override;
		void CommandPoolReset(CommandPoolRH InPool) override;
		CommandListRH CommandPoolNewList(CommandPoolRH InPool) override;
		void CommandPoolDestroy(CommandPoolRH InPool) override;

		void CommandListBegin(CommandListRH InList) override;
		void CommandListImageBarrier(CommandListRH InList, ImageBarrier InBarrier) override;
		void CommandListBeginRendering(CommandListRH InList, RenderTarget InRenderTarget) override;
		void CommandListEndRendering(CommandListRH InList) override;
		void CommandListCopyBuffer(CommandListRH InList, BufferRH InDest, BufferRH InSrc) override;
		void CommandListPushConstants(CommandListRH InList, PipelineRH InPipeline, ShaderStage InStages, const void* InData, uint32_t InDataSize) override;
		void CommandListBindDescriptorSets(CommandListRH InList, PipelineRH InPipeline, Span<DescriptorSetRH> InDescriptorSets) override;
		void CommandListBindPipeline(CommandListRH InList, PipelineRH InPipeline) override;
		void CommandListTraceRay(CommandListRH InList, PipelineRH InPipeline, uint32_t InWidth, uint32_t InHeight) override;
		void CommandListEnd(CommandListRH InList) override;

		ImageRH ImageCreate(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) override;
		void ImageDestroy(ImageRH InImage) override;

		ImageViewRH ImageViewCreate(ImageRH InImage) override;
		void ImageViewDestroy(ImageViewRH InImageView) override;

		BufferRH BufferCreate(uint64_t InSize, BufferUsage InType, bool InHostAccess = false) override;
		void BufferSetData(BufferRH InBuffer, const void* InData, uint64_t InDataSize) override;
		uint64_t BufferGetDeviceAddress(BufferRH InBuffer) override;
		void* BufferGetMappedMemory(BufferRH InBuffer) override;
		void BufferDestroy(BufferRH InBuffer) override;

		DescriptorSetLayoutRH DescriptorSetLayoutCreate(const DescriptorSetLayoutInfo& InLayoutInfo) override;
		void DescriptorSetLayoutDestroy(DescriptorSetLayoutRH InLayout) override;
		DescriptorPoolRH DescriptorPoolCreate(Span<DescriptorCount> InDescriptorCounts) override;
		void DescriptorPoolDestroy(DescriptorPoolRH InPool) override;
		DescriptorSetRH DescriptorPoolAllocateDescriptorSet(DescriptorPoolRH InPool, DescriptorSetLayoutRH InLayout) override;
		void DescriptorSetWrite(DescriptorSetRH InSet, uint32_t InBinding, Span<ImageViewRH> InImageViews, uint32_t InArrayOffset) override;

		PipelineRH PipelineCreate(const PipelineInfo& InPipelineInfo) override;
		void PipelineDestroy(PipelineRH InPipeline) override;

		AccelerationStructureRH AccelerationStructureCreate(BufferRH InVertexBuffer, BufferRH InIndexBuffer) override;
		uint64_t AccelerationStructureGetTopLevelAddress(AccelerationStructureRH InAccelerationStructure) override;
		void AccelerationStructureDestroy(AccelerationStructureRH InAccelerationStructure) override;

	private:
		VulkanRenderDevice(VkInstance InInstance, const DynamicArray<RendererFeature>& InRequestedFeatures);

		void FindPhysicalDevice();
		void CreateLogicalDevice();
		void RemoveUnsupportedFeatures();

	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;

		VmaAllocator m_Allocator = VK_NULL_HANDLE;

		HashMap<RendererFeature, Unique<VulkanFeature>> m_Features;

		VulkanShaderCompiler m_ShaderCompiler;

	private:
		ResourceRegistry<QueueRH, VulkanQueue> m_Queues;
		ResourceRegistry<SwapchainRH, VulkanSwapchain> m_Swapchains;
		ResourceRegistry<FenceRH, VulkanFence> m_Fences;
		ResourceRegistry<CommandPoolRH, VulkanCommandPool> m_CommandPools;
		ResourceRegistry<CommandListRH, VulkanCommandList> m_CommandLists;
		ResourceRegistry<ImageRH, VulkanImage> m_Images;
		ResourceRegistry<ImageViewRH, VulkanImageView> m_ImageViews;
		ResourceRegistry<BufferRH, VulkanBuffer> m_Buffers;
		ResourceRegistry<PipelineRH, VulkanPipeline> m_Pipelines;
		ResourceRegistry<DescriptorSetLayoutRH, VulkanDescriptorSetLayout> m_DescriptorSetLayouts;
		ResourceRegistry<DescriptorPoolRH, VulkanDescriptorPool> m_DescriptorPools;
		ResourceRegistry<DescriptorSetRH, VulkanDescriptorSet> m_DescriptorSets;
		ResourceRegistry<AccelerationStructureRH, VulkanAccelerationStructure> m_AccelerationStructures;

	private:
		friend class VulkanContext;
	};

}
