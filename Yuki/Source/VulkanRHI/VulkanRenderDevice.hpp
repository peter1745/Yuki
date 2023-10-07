#pragma once

namespace Yuki::RHI {

	//class VulkanRenderDevice final : public RenderDevice
	//{
	//public:
	//	bool IsFeatureEnabled(RendererFeature InFeature) const override;

	//	template<std::derived_from<VulkanFeature> TFeature>
	//	TFeature& GetFeature() const
	//	{
	//		return m_Features.at(TFeature::GetRendererFeature());
	//	}

	//public:
	//	QueueRH QueueRequest(QueueType InType) override;

	//	SwapchainRH SwapchainCreate(const WindowSystem& InWindowSystem, UniqueID InWindowHandle) override;
	//	void SwapchainDestroy(SwapchainRH InSwapchain) override;
	//	//void SwapchainRecreate(VulkanSwapchain& InSwapchain);

	//	FenceRH FenceCreate() override;
	//	void FenceDestroy(FenceRH InFence) override;

	//	CommandPoolRH CommandPoolCreate(QueueRH InQueue) override;
	//	void CommandPoolDestroy(CommandPoolRH InPool) override;

	//	ImageRH ImageCreate(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) override;
	//	void ImageDestroy(ImageRH InImage) override;

	//	ImageViewRH ImageViewCreate(ImageRH InImage) override;
	//	void ImageViewDestroy(ImageViewRH InImageView) override;

	//	BufferRH BufferCreate(uint64_t InSize, BufferUsage InType, bool InHostAccess = false) override;
	//	void BufferDestroy(BufferRH InBuffer) override;

	//	DescriptorSetLayoutRH DescriptorSetLayoutCreate(const DescriptorSetLayoutInfo& InLayoutInfo) override;
	//	void DescriptorSetLayoutDestroy(DescriptorSetLayoutRH InLayout) override;
	//	DescriptorPoolRH DescriptorPoolCreate(Span<DescriptorCount> InDescriptorCounts) override;
	//	void DescriptorPoolDestroy(DescriptorPoolRH InPool) override;

	//	PipelineRH PipelineCreate(const PipelineInfo& InPipelineInfo) override;
	//	void PipelineDestroy(PipelineRH InPipeline) override;

	//	RayTracingPipelineRH RayTracingPipelineCreate(const RayTracingPipelineInfo& InPipelineInfo) override;
	//	void RayTracingPipelineDestroy(RayTracingPipelineRH InPipeline) override;

	//	AccelerationStructureRH AccelerationStructureCreate(BufferRH InVertexBuffer, BufferRH InIndexBuffer) override;
	//	void AccelerationStructureDestroy(AccelerationStructureRH InAccelerationStructure) override;

	//private:
	//	VulkanRenderDevice(VkInstance InInstance, const DynamicArray<RendererFeature>& InRequestedFeatures);

	//	void FindPhysicalDevice();
	//	void CreateLogicalDevice();
	//	void RemoveUnsupportedFeatures();

	//private:
	//	VkInstance m_Instance = VK_NULL_HANDLE;
	//	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	//	VkDevice m_Device = VK_NULL_HANDLE;

	//	VmaAllocator m_Allocator = VK_NULL_HANDLE;

	//	HashMap<RendererFeature, Unique<VulkanFeature>> m_Features;
	//	DynamicArray<QueueRH> m_Queues;

	//	VulkanShaderCompiler m_ShaderCompiler;

	//private:
	//	friend class VulkanContext;
	//};

}
