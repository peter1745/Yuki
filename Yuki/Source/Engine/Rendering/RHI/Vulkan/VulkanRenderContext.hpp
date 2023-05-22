#pragma once

#include "Rendering/RHI/RenderContext.hpp"

#include "Vulkan.hpp"
#include "VulkanPlatform.hpp"
#include "VulkanShaderCompiler.hpp"
#include "VulkanQueue.hpp"

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

	public:
		Swapchain* CreateSwapchain(GenericWindow* InWindow) override;
		Image2D* CreateImage2D(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat) override;
		ImageView2D* CreateImageView2D(Image2D* InImage) override;
		RenderTarget* CreateRenderTarget(const RenderTargetInfo& InInfo) override;

		void DestroySwapchain(Swapchain* InSwapchain) override;
		void DestroyImage2D(Image2D* InImage) override;
		void DestroyImageView2D(ImageView2D* InImageView) override;
		void DestroyRenderTarget(RenderTarget* InRenderTarget) override;

	public:
		VkInstance GetInstance() const { return m_Instance; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkDevice GetDevice() const { return m_Device; }

		VulkanQueue* GetGraphicsQueue() const { return m_GraphicsQueue.GetPtr(); }

		VkSurfaceCapabilitiesKHR QuerySurfaceCapabilities(VkSurfaceKHR InSurface) const;

	private:
		bool HasValidationLayerSupport() const;
		void SelectSuitablePhysicalDevice();
		void CreateLogicalDevice(const List<const char*>& InDeviceLayers);

	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		Unique<VulkanQueue> m_GraphicsQueue = nullptr;

		Unique<ShaderManager> m_ShaderManager = nullptr;
		Unique<VulkanShaderCompiler> m_ShaderCompiler = nullptr;
	};

}
