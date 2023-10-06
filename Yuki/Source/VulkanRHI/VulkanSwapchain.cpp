#include "VulkanSwapchain.hpp"
#include "VulkanRenderDevice.hpp"
#include "VulkanUtils.hpp"
#include "VulkanPlatformUtils.hpp"
#include "VulkanImage.hpp"

namespace Yuki::RHI {

	void VulkanRenderDevice::SwapchainRecreate(VulkanSwapchain& InSwapchain)
	{
		auto OldSwapchain = InSwapchain.Handle;

		if (OldSwapchain != VK_NULL_HANDLE)
		{
			for (auto ImageView : InSwapchain.ImageViews)
				ImageViewDestroy(ImageView);
		}

		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, InSwapchain.Surface, &SurfaceCapabilities);

		DynamicArray<VkSurfaceFormatKHR> SurfaceFormats;
		Vulkan::Enumerate(vkGetPhysicalDeviceSurfaceFormatsKHR, SurfaceFormats, m_PhysicalDevice, InSwapchain.Surface);
		for (auto Format : SurfaceFormats)
		{
			if (Format.format == VK_FORMAT_R8G8B8A8_UNORM || Format.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				InSwapchain.SurfaceFormat = Format;
				break;
			}
		}

		uint32_t Width = SurfaceCapabilities.currentExtent.width;
		uint32_t Height = SurfaceCapabilities.currentExtent.height;
		if (Width == std::numeric_limits<uint32_t>::max())
		{
			const auto& WindowData = InSwapchain.WindowingSystem->GetWindowData(InSwapchain.TargetWindow);
			Width = std::clamp(WindowData.Width, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width);
			Height = std::clamp(WindowData.Height, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height);
		}

		VkSwapchainCreateInfoKHR SwapchainInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.surface = InSwapchain.Surface,
			.minImageCount = std::min(SurfaceCapabilities.minImageCount + 1, SurfaceCapabilities.maxImageCount),
			.imageFormat = InSwapchain.SurfaceFormat.format,
			.imageColorSpace = InSwapchain.SurfaceFormat.colorSpace,
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
		YUKI_VERIFY(vkCreateSwapchainKHR(m_Device, &SwapchainInfo, nullptr, &InSwapchain.Handle) == VK_SUCCESS);

		for (auto ImageHandle : InSwapchain.Images)
		{
			auto& Image = m_Images[ImageHandle];
			Image.Handle = VK_NULL_HANDLE;
			m_Images.Return(ImageHandle);
		}

		DynamicArray<VkImage> Images;
		Vulkan::Enumerate(vkGetSwapchainImagesKHR, Images, m_Device, InSwapchain.Handle);

		InSwapchain.Images.resize(Images.size());

		for (size_t Index = 0; Index < Images.size(); Index++)
		{
			auto[ImageHandle, ImageData] = m_Images.Acquire();
			ImageData.Handle = Images[Index];
			ImageData.Width = Width;
			ImageData.Height = Height;
			ImageData.Format = InSwapchain.SurfaceFormat.format;
			ImageData.AspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ImageData.Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			ImageData.OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			InSwapchain.Images[Index] = ImageHandle;
		}

		InSwapchain.ImageViews.resize(InSwapchain.Images.size());
		for (size_t Index = 0; Index < InSwapchain.Images.size(); Index++)
			InSwapchain.ImageViews[Index] = ImageViewCreate(InSwapchain.Images[Index]);

		while (InSwapchain.Semaphores.size() < InSwapchain.Images.size() * 2)
		{
			VkSemaphoreCreateInfo SemaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			VkSemaphore Semaphore;
			YUKI_VERIFY(vkCreateSemaphore(m_Device, &SemaphoreInfo, nullptr, &Semaphore) == VK_SUCCESS);
			InSwapchain.Semaphores.push_back(Semaphore);
		}

		if (OldSwapchain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(m_Device, OldSwapchain, nullptr);
	}

	SwapchainRH VulkanRenderDevice::SwapchainCreate(const WindowSystem& InWindowSystem, UniqueID InWindowHandle)
	{
		auto[Handle, Swapchain] = m_Swapchains.Acquire();
		Swapchain.WindowingSystem = &InWindowSystem;
		Swapchain.TargetWindow = InWindowHandle;
		Swapchain.Surface = Vulkan::CreateSurface(m_Instance, InWindowSystem, InWindowHandle);

		SwapchainRecreate(Swapchain);

		return Handle;
	}

	ImageRH VulkanRenderDevice::SwapchainGetCurrentImage(SwapchainRH InSwapchain)
	{
		const auto& Swapchain = m_Swapchains[InSwapchain];
		return Swapchain.Images[Swapchain.CurrentImageIndex];
	}

	ImageViewRH VulkanRenderDevice::SwapchainGetCurrentImageView(SwapchainRH InSwapchain)
	{
		const auto& Swapchain = m_Swapchains[InSwapchain];
		return Swapchain.ImageViews[Swapchain.CurrentImageIndex];
	}

	void VulkanRenderDevice::SwapchainDestroy(SwapchainRH InSwapchain)
	{

	}

}
