#pragma once

#include "Core/GenericWindow.hpp"
#include "Rendering/RHI/Swapchain.hpp"
#include "Rendering/RHI/CommandBuffer.hpp"

#include "VulkanRenderContext.hpp"
#include "VulkanImage2D.hpp"

namespace Yuki {

	struct VulkanSwapchainInfo
	{
		VkSurfaceKHR Surface;
		VkSurfaceFormatKHR SurfaceFormat;
		VkPresentModeKHR PresentMode;
		uint32_t MinImageCount;
		VkExtent2D ImageExtent;
		VkSurfaceTransformFlagBitsKHR PreTransform;
	};

	class VulkanSwapchain : public Swapchain
	{
	public:
		VulkanSwapchain(VulkanRenderContext* InContext, const VulkanSwapchainInfo& InSwapchainInfo);

		void Destroy() override;

		VkSwapchainKHR GetVkSwapchain() const { return m_Swapchain; }

		Image2D* GetCurrentImage() const override { return m_Images[m_CurrentImageIndex]; }
		ImageView2D* GetCurrentImageView() const override { return m_ImageViews[m_CurrentImageIndex]; }

	private:
		void Create(const VulkanSwapchainInfo& InSwapchainInfo);
		void Recreate(const VulkanSwapchainInfo& InSwapchainInfo);

		VkResult AcquireNextImage();

	private:
		VulkanRenderContext* m_Context = nullptr;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		List<VulkanImage2D*> m_Images;
		List<VulkanImageView2D*> m_ImageViews;
		uint32_t m_CurrentImageIndex = 0;

		List<VkSemaphore> m_Semaphores;
		uint32_t m_SemaphoreIndex = 0;

		friend class VulkanRenderContext;
		friend class VulkanQueue;
		friend class VulkanViewport;
	};

}
