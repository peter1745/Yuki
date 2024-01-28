#include "VulkanRHI.hpp"

#include <Engine/Core/Window.hpp>

// NOTE(Peter): This should preferably be in a file that only gets compiled on Windows
#if defined(YUKI_PLATFORM_WINDOWS)
	#include <Platform/Windows/WindowImpl.hpp>
#endif

namespace Yuki {

	Swapchain Swapchain::Create(RHIContext context, Window window)
	{
		auto* impl = new Impl();
		impl->Context = context;

		// Initialize data that persists after a resize

#if defined(YUKI_PLATFORM_WINDOWS)
		VkWin32SurfaceCreateInfoKHR surfaceInfo =
		{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.hinstance = GetModuleHandle(nullptr),
			.hwnd = window->WindowHandle,
		};
		Vulkan::CheckResult(vkCreateWin32SurfaceKHR(context->Instance, &surfaceInfo, nullptr, &impl->Surface));
#endif

		impl->Recreate(window);

		return { impl };
	}

	void Swapchain::Impl::Recreate(Window window)
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		Vulkan::CheckResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			Context->PhysicalDevice,
			Surface,
			&surfaceCapabilities
		));

		std::vector<VkSurfaceFormatKHR> availableFormats;
		Vulkan::Enumerate(vkGetPhysicalDeviceSurfaceFormatsKHR, availableFormats, Context->PhysicalDevice, Surface);
		const auto& surfaceFormat = std::ranges::find_if(availableFormats, [](const auto& availableFormat)
		{
			return availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM || availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM;
		});

		/*VkSwapchainCreateInfoKHR swapchainInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.surface = Surface,
			.minImageCount,
			.imageFormat,
			.imageColorSpace,
			.imageExtent,
			.imageArrayLayers,
			.imageUsage,
			.imageSharingMode,
			.queueFamilyIndexCount,
			.pQueueFamilyIndices,
			.preTransform,
			.compositeAlpha,
			.presentMode,
			.clipped,
			.oldSwapchain,
		};*/
	}

}
