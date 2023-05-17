#pragma once

#include "Core/GenericWindow.hpp"
#include "Rendering/Swapchain.hpp"

#include "VulkanDevice.hpp"

namespace Yuki {

	class VulkanSwapchain : public Swapchain
	{
	public:
		VulkanSwapchain(GenericWindow* InWindow, VkInstance InInstance, VulkanDevice* InDevice);

		bool AcquireNextImage(VulkanDevice* InDevice, VkSemaphore InSemaphore);

		void Destroy(VulkanDevice* InDevice);

	private:
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		List<VkImage> m_Images;
		List<VkImageView> m_ImageViews;
		uint32_t m_CurrentImage = 0;
	};

}
