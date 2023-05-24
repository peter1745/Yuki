#pragma once

#include "Rendering/RHI/RenderContext.hpp"

#include "Vulkan.hpp"
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

		void ResetCommandPool() override;

		void WaitDeviceIdle() const override;

	public:
		Viewport* CreateViewport(GenericWindow* InWindow) override;
		Image2D* CreateImage2D(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat) override;
		ImageView2D* CreateImageView2D(Image2D* InImage) override;
		RenderTarget* CreateRenderTarget(const RenderTargetInfo& InInfo) override;
		Fence* CreateFence() override;
		CommandBuffer CreateCommandBuffer() override;

		void DestroyFence(Fence* InFence) override;
		void DestroyViewport(Viewport* InViewport) override;
		void DestroyImage2D(Image2D* InImage) override;
		void DestroyImageView2D(ImageView2D* InImageView) override;
		void DestroyRenderTarget(RenderTarget* InRenderTarget) override;

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

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;

		Unique<ShaderManager> m_ShaderManager = nullptr;
		Unique<VulkanShaderCompiler> m_ShaderCompiler = nullptr;
	};

}
