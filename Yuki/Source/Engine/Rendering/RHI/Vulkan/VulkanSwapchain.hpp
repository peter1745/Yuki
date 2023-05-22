#pragma once

#include "Core/GenericWindow.hpp"
#include "Rendering/RHI/Swapchain.hpp"

#include "VulkanRenderContext.hpp"

namespace Yuki {

	class VulkanSwapchain : public Swapchain
	{
	private:
		static VulkanSwapchain* Create(VulkanRenderContext* InContext, GenericWindow* InWindow);
		static void Destroy(VulkanRenderContext* InContext, VulkanSwapchain* InSwapchain);

	private:
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		List<VkImage> m_Images;
		List<VkImageView> m_ImageViews;
		uint32_t m_CurrentImage = 0;

		friend class VulkanRenderContext;
	};

}
