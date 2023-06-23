#pragma once

#include "VulkanInclude.hpp"
#include "Rendering/RHI.hpp"

namespace Yuki {

	class GenericWindow;

	struct VulkanSwapchain
	{
		GenericWindow* Window = nullptr;

		VkSurfaceKHR Surface = VK_NULL_HANDLE;
		VkSurfaceFormatKHR SurfaceFormat = {};

		uint32_t Width = 0;
		uint32_t Height = 0;

		// Use FIFO in case MAILBOX isn't available (FIFO is the only present mode that is guaranteed to be supported)
		VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;

		VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
		
		DynamicArray<Image> Images;
		DynamicArray<ImageView> ImageViews;
		uint32_t CurrentImage = 0;

		DynamicArray<VkSemaphore> Semaphores;
		uint32_t SemaphoreIndex = 0;
	};

}
