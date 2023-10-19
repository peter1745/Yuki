#include "VulkanRHI.hpp"
#include "VulkanUtils.hpp"
#include "VulkanPlatformUtils.hpp"

#include "Engine/Common/WindowSystem.hpp"

namespace Yuki::RHI {

	void Swapchain::Recreate() const
	{
		auto oldSwapchain = m_Impl->Handle;

		if (oldSwapchain != VK_NULL_HANDLE)
		{
			for (auto imageView : m_Impl->ImageViews)
				imageView.Destroy();
		}

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Impl->Ctx->PhysicalDevice, m_Impl->Surface, &surfaceCapabilities);

		DynamicArray<VkSurfaceFormatKHR> surfaceFormats;
		Vulkan::Enumerate(vkGetPhysicalDeviceSurfaceFormatsKHR, surfaceFormats, m_Impl->Ctx->PhysicalDevice, m_Impl->Surface);
		for (auto format : surfaceFormats)
		{
			if (format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				m_Impl->SurfaceFormat = format;
				break;
			}
		}

		uint32_t width = surfaceCapabilities.currentExtent.width;
		uint32_t height = surfaceCapabilities.currentExtent.height;
		if (width == std::numeric_limits<uint32_t>::max())
		{
			const auto& windowData = m_Impl->WindowingSystem->GetWindowData(m_Impl->TargetWindow);
			width = std::clamp(windowData.Width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			height = std::clamp(windowData.Height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}

		VkSwapchainCreateInfoKHR swapchainInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.surface = m_Impl->Surface,
			.minImageCount = std::min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount),
			.imageFormat = m_Impl->SurfaceFormat.format,
			.imageColorSpace = m_Impl->SurfaceFormat.colorSpace,
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
			.oldSwapchain = oldSwapchain,
		};
		YUKI_VK_CHECK(vkCreateSwapchainKHR(m_Impl->Ctx->Device, &swapchainInfo, nullptr, &m_Impl->Handle));

		for (auto imageHandle : m_Impl->Images)
		{
			delete imageHandle.m_Impl;
		}

		DynamicArray<VkImage> images;
		Vulkan::Enumerate(vkGetSwapchainImagesKHR, images, m_Impl->Ctx->Device, m_Impl->Handle);

		m_Impl->Images.resize(images.size());

		for (size_t i = 0; i < images.size(); i++)
		{
			auto imageData = new Image::Impl();
			imageData->Handle = images[i];
			imageData->Width = width;
			imageData->Height = height;
			imageData->Format = m_Impl->SurfaceFormat.format;
			imageData->AspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageData->Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageData->OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageData->DefaultView = ImageView::Create(m_Impl->Ctx, imageData);
			m_Impl->Images[i] = { imageData };
		}

		m_Impl->ImageViews.resize(m_Impl->Images.size());
		for (size_t i = 0; i < m_Impl->Images.size(); i++)
		{
			m_Impl->ImageViews[i] = ImageView::Create(m_Impl->Ctx, m_Impl->Images[i]);
		}

		while (m_Impl->Semaphores.size() < m_Impl->Images.size() * 2)
		{
			VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			VkSemaphore semaphore;
			YUKI_VK_CHECK(vkCreateSemaphore(m_Impl->Ctx->Device, &semaphoreInfo, nullptr, &semaphore));
			m_Impl->Semaphores.push_back(semaphore);
		}

		if (oldSwapchain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(m_Impl->Ctx->Device, oldSwapchain, nullptr);
	}

	Swapchain Swapchain::Create(Context context, const WindowSystem& windowSystem, UniqueID windowHandle)
	{
		auto swapchain = new Swapchain::Impl();
		swapchain->Ctx = context;
		swapchain->WindowingSystem = &windowSystem;
		swapchain->TargetWindow = windowHandle;
		swapchain->Surface = Vulkan::CreateSurface(context->Instance, windowSystem, windowHandle);

		Swapchain result = { swapchain };
		result.Recreate();
		return result;
	}

	ImageRH Swapchain::GetCurrentImage()
	{
		return m_Impl->Images[m_Impl->CurrentImageIndex];
	}

	ImageViewRH Swapchain::GetCurrentImageView()
	{
		return m_Impl->ImageViews[m_Impl->CurrentImageIndex];
	}

	/*void VulkanRenderDevice::SwapchainDestroy(SwapchainRH InSwapchain)
	{

	}*/

}
