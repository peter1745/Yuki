#include "VulkanRHI.hpp"

// NOTE(Peter): This should preferably be in a file that only gets compiled on Windows
#if defined(YUKI_PLATFORM_WINDOWS)
	#include <Platform/Windows/WindowImpl.hpp>
#endif

namespace Yuki {

	Swapchain Swapchain::Create(RHIContext context, Window window)
	{
		auto* impl = new Impl();
		impl->Context = context;
		impl->Target = window;

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

		impl->Recreate();

		return { impl };
	}

	void Swapchain::Impl::Recreate()
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		Vulkan::CheckResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			Context->PhysicalDevice,
			Surface,
			&surfaceCapabilities
		));

		std::vector<VkSurfaceFormatKHR> availableFormats;
		Vulkan::Enumerate(vkGetPhysicalDeviceSurfaceFormatsKHR, availableFormats, Context->PhysicalDevice, Surface);
		const auto& surfaceFormat = *std::ranges::find_if(availableFormats, [](const auto& availableFormat)
		{
			return availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM || availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM;
		});

		uint32_t width = surfaceCapabilities.currentExtent.width;
		uint32_t height = surfaceCapabilities.currentExtent.height;

		if (width == std::numeric_limits<uint32_t>::max())
		{
			width = std::clamp(Target.GetWidth(), surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			height = std::clamp(Target.GetHeight(), surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}

		VkSwapchainCreateInfoKHR swapchainInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.surface = Surface,
			.minImageCount = std::min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount),
			.imageFormat = surfaceFormat.format,
			.imageColorSpace = surfaceFormat.colorSpace,
			.imageExtent = { width, height },
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.preTransform = surfaceCapabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
			.clipped = VK_TRUE,
			.oldSwapchain = nullptr,
		};
		Vulkan::CheckResult(vkCreateSwapchainKHR(Context->Device, &swapchainInfo, nullptr, &Resource));

		std::vector<VkImage> swapchainImages;
		Vulkan::Enumerate(vkGetSwapchainImagesKHR, swapchainImages, Context->Device, Resource);
		for (auto image : swapchainImages)
		{
			auto* imageImpl = new Image::Impl();
			imageImpl->Resource = image;
			imageImpl->Width = width;
			imageImpl->Height = height;
			imageImpl->Format = surfaceFormat.format;
			imageImpl->OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageImpl->Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageImpl->AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

			Images.push_back({ imageImpl });
			ImageViews.push_back(ImageView::Create(Context, { imageImpl }));
		}

		while (Semaphores.size() < Images.size() * 2)
		{
			VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			VkSemaphore semaphore;
			Vulkan::CheckResult(vkCreateSemaphore(Context->Device, &semaphoreInfo, nullptr, &semaphore));
			Semaphores.push_back(semaphore);
		}
	}

	void Swapchain::Destroy()
	{
		for (auto semaphore : m_Impl->Semaphores)
		{
			vkDestroySemaphore(m_Impl->Context->Device, semaphore, nullptr);
		}
		m_Impl->Semaphores.clear();

		if (m_Impl->Resource)
		{
			vkDestroySwapchainKHR(m_Impl->Context->Device, m_Impl->Resource, nullptr);
			vkDestroySurfaceKHR(m_Impl->Context->Instance, m_Impl->Surface, nullptr);
		}

		delete m_Impl;
	}

	Image Swapchain::GetCurrentImage() const { return m_Impl->Images[m_Impl->CurrentImageIndex]; }
	ImageView Swapchain::GetCurrentImageView() const { return m_Impl->ImageViews[m_Impl->CurrentImageIndex]; }

}
