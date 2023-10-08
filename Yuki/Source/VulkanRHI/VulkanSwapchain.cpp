#include "VulkanRHI.hpp"
#include "VulkanUtils.hpp"
#include "VulkanPlatformUtils.hpp"

#include "Engine/Common/WindowSystem.hpp"

namespace Yuki::RHI {

	void Swapchain::Recreate() const
	{
		auto OldSwapchain = m_Impl->Handle;

		if (OldSwapchain != VK_NULL_HANDLE)
		{
			for (auto ImageView : m_Impl->ImageViews)
				ImageView.Destroy();
		}

		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Impl->Ctx->PhysicalDevice, m_Impl->Surface, &SurfaceCapabilities);

		DynamicArray<VkSurfaceFormatKHR> SurfaceFormats;
		Vulkan::Enumerate(vkGetPhysicalDeviceSurfaceFormatsKHR, SurfaceFormats, m_Impl->Ctx->PhysicalDevice, m_Impl->Surface);
		for (auto Format : SurfaceFormats)
		{
			if (Format.format == VK_FORMAT_R8G8B8A8_UNORM || Format.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				m_Impl->SurfaceFormat = Format;
				break;
			}
		}

		uint32_t Width = SurfaceCapabilities.currentExtent.width;
		uint32_t Height = SurfaceCapabilities.currentExtent.height;
		if (Width == std::numeric_limits<uint32_t>::max())
		{
			const auto& WindowData = m_Impl->WindowingSystem->GetWindowData(m_Impl->TargetWindow);
			Width = std::clamp(WindowData.Width, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width);
			Height = std::clamp(WindowData.Height, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height);
		}

		VkSwapchainCreateInfoKHR SwapchainInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.surface = m_Impl->Surface,
			.minImageCount = std::min(SurfaceCapabilities.minImageCount + 1, SurfaceCapabilities.maxImageCount),
			.imageFormat = m_Impl->SurfaceFormat.format,
			.imageColorSpace = m_Impl->SurfaceFormat.colorSpace,
			.imageExtent = { Width, Height },
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
			.preTransform = SurfaceCapabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
			.clipped = VK_TRUE,
			.oldSwapchain = OldSwapchain,
		};
		YUKI_VERIFY(vkCreateSwapchainKHR(m_Impl->Ctx->Device, &SwapchainInfo, nullptr, &m_Impl->Handle) == VK_SUCCESS);

		for (auto ImageHandle : m_Impl->Images)
		{
			delete ImageHandle.m_Impl;
		}

		DynamicArray<VkImage> Images;
		Vulkan::Enumerate(vkGetSwapchainImagesKHR, Images, m_Impl->Ctx->Device, m_Impl->Handle);

		m_Impl->Images.resize(Images.size());

		for (size_t Index = 0; Index < Images.size(); Index++)
		{
			auto ImageData = new Image::Impl();
			ImageData->Handle = Images[Index];
			ImageData->Width = Width;
			ImageData->Height = Height;
			ImageData->Format = m_Impl->SurfaceFormat.format;
			ImageData->AspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ImageData->Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			ImageData->OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			m_Impl->Images[Index] = { ImageData };
		}

		m_Impl->ImageViews.resize(m_Impl->Images.size());
		for (size_t Index = 0; Index < m_Impl->Images.size(); Index++)
		{
			m_Impl->ImageViews[Index] = ImageView::Create(m_Impl->Ctx, m_Impl->Images[Index]);
		}

		while (m_Impl->Semaphores.size() < m_Impl->Images.size() * 2)
		{
			VkSemaphoreCreateInfo SemaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			VkSemaphore Semaphore;
			YUKI_VERIFY(vkCreateSemaphore(m_Impl->Ctx->Device, &SemaphoreInfo, nullptr, &Semaphore) == VK_SUCCESS);
			m_Impl->Semaphores.push_back(Semaphore);
		}

		if (OldSwapchain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(m_Impl->Ctx->Device, OldSwapchain, nullptr);
	}

	Swapchain Swapchain::Create(Context InContext, const WindowSystem& InWindowSystem, UniqueID InWindowHandle)
	{
		auto swapchain = new Swapchain::Impl();
		swapchain->Ctx = InContext;
		swapchain->WindowingSystem = &InWindowSystem;
		swapchain->TargetWindow = InWindowHandle;
		swapchain->Surface = Vulkan::CreateSurface(InContext->Instance, InWindowSystem, InWindowHandle);

		Swapchain Result = { swapchain };
		Result.Recreate();
		return Result;
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
