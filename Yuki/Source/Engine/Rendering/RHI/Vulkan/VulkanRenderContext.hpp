#pragma once

#include "Rendering/RHI/RenderContext.hpp"

#include "VulkanInclude.hpp"
#include "VulkanPlatform.hpp"
#include "VulkanShaderCompiler.hpp"
#include "VulkanQueue.hpp"
#include "VulkanAllocator.hpp"

namespace Yuki {

	class VulkanRenderContext : public RenderContext
	{
	public:
		VulkanRenderContext();
		~VulkanRenderContext() = default;

		void Initialize() override;
		void Destroy() override;

		ShaderManager* GetShaderManager() const override { return m_ShaderManager.GetPtr(); }
		ShaderCompiler* GetShaderCompiler() const override { return m_ShaderCompiler.GetPtr(); }

		Queue* GetGraphicsQueue() const override { return m_GraphicsQueue.GetPtr(); }

		void WaitDeviceIdle() const override;

	public:
		Unique<RenderInterface> CreateRenderInterface() override;
		Unique<GraphicsPipelineBuilder> CreateGraphicsPipelineBuilder() override;
		Unique<SetLayoutBuilder> CreateSetLayoutBuilder() override;
		Unique<DescriptorPool> CreateDescriptorPool(std::span<DescriptorCount> InDescriptorCounts) override;
		Unique<Viewport> CreateViewport(GenericWindow* InWindow) override;
		Unique<Image2D> CreateImage2D(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) override;
		Unique<ImageView2D> CreateImageView2D(Image2D* InImage) override;
		Unique<Sampler> CreateSampler() override;
		Unique<Buffer> CreateBuffer(const BufferInfo& InInfo) override;
		Unique<Fence> CreateFence() override;
		Unique<CommandBufferPool> CreateCommandBufferPool(CommandBufferPoolInfo InInfo) override;

	public:
		VulkanAllocator& GetAllocator() { return m_Allocator; }

		VkInstance GetInstance() const { return m_Instance; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkDevice GetDevice() const { return m_Device; }

		VkSurfaceCapabilitiesKHR QuerySurfaceCapabilities(VkSurfaceKHR InSurface) const;

	private:
		bool HasValidationLayerSupport() const;
		void SelectSuitablePhysicalDevice();
		void CreateLogicalDevice(const List<const char*>& InDeviceLayers);

		void SetupDebugUtilsMessenger();

	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		Unique<VulkanQueue> m_GraphicsQueue = nullptr;
		VulkanAllocator m_Allocator;

		VkDebugUtilsMessengerEXT m_DebugUtilsMessengerHandle = VK_NULL_HANDLE;

		Unique<ShaderManager> m_ShaderManager = nullptr;
		Unique<VulkanShaderCompiler> m_ShaderCompiler = nullptr;
	};

}
