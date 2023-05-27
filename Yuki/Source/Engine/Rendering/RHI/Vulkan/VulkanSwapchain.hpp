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

		void BeginRendering(CommandBuffer* InCmdBuffer) override;
		void EndRendering(CommandBuffer* InCmdBuffer) override;

		VkSwapchainKHR GetVkSwapchain() const { return m_Swapchain; }
		uint32_t GetCurrentImageIndex() const { return m_CurrentImage; }

		uint32_t& GetSemaphoreIndex() { return m_SemaphoreIndex; }
		uint32_t GetSemaphoreCount() const { return uint32_t(m_Semaphores.size()); }
		VkSemaphore GetSemaphore(uint32_t InIndex) const { return m_Semaphores[InIndex]; }

	private:
		void Create(const VulkanSwapchainInfo& InSwapchainInfo);
		void Recreate(const VulkanSwapchainInfo& InSwapchainInfo);

		VkResult AcquireNextImage();

		uint32_t& GetCurrentImageIndex() { return m_CurrentImage; }

	private:
		VulkanRenderContext* m_Context = nullptr;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		List<VulkanImage2D*> m_Images;
		List<VulkanImageView2D*> m_ImageViews;
		uint32_t m_CurrentImage = 0;

		List<VkSemaphore> m_Semaphores;
		uint32_t m_SemaphoreIndex = 0;

		friend class VulkanRenderContext;
		friend class VulkanQueue;
		friend class VulkanViewport;
	};

}
