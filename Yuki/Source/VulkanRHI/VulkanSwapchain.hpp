#pragma once

#include "Engine/Common/WindowSystem.hpp"
#include "Engine/RHI/RenderHandles.hpp"

#include "VulkanInclude.hpp"

namespace Yuki::RHI {

	struct VulkanSwapchain
	{
		VkSwapchainKHR Handle = VK_NULL_HANDLE;
		VkSurfaceKHR Surface = VK_NULL_HANDLE;
		VkSurfaceFormatKHR SurfaceFormat = {};
		DynamicArray<ImageRH> Images;
		DynamicArray<ImageViewRH> ImageViews;
		uint32_t CurrentImageIndex = 0;
		DynamicArray<VkSemaphore> Semaphores;
		uint32_t CurrentSemaphoreIndex = 0;

		const WindowSystem* WindowingSystem;
		WindowHandle TargetWindow;
	};

}
