#include "VulkanSwapchain.hpp"
#include "VulkanRenderContext.hpp"
#include "VulkanPlatform.hpp"
#include "VulkanHelper.hpp"

#include "Math/Math.hpp"

namespace Yuki {

	SwapchainHandle VulkanRenderContext::CreateSwapchain(GenericWindow* InWindow)
	{
		auto[handle, swapchain] = m_Swapchains.Acquire();

		swapchain.Window = InWindow;

		RecreateSwapchain(swapchain);
		return handle;
	}

	void VulkanRenderContext::RecreateSwapchain(VulkanSwapchain& InSwapchain)
	{
		VkSwapchainKHR oldSwapchain = InSwapchain.Swapchain;
		ImageHandle oldDepthImage = InSwapchain.DepthImage;

		if (oldSwapchain != VK_NULL_HANDLE)
		{
			for (auto imageView : InSwapchain.ImageViews)
				Destroy(imageView);
		}

		if (InSwapchain.Surface == VK_NULL_HANDLE)
			InSwapchain.Surface = VulkanPlatform::CreateSurface(m_Instance, InSwapchain.Window);

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, InSwapchain.Surface, &surfaceCapabilities);

		// Find suitable surface format
		{
			DynamicArray<VkSurfaceFormatKHR> availableSurfaceFormats;
			VulkanHelper::Enumerate(vkGetPhysicalDeviceSurfaceFormatsKHR, availableSurfaceFormats, m_PhysicalDevice, InSwapchain.Surface);

			for (const auto& surfaceFormat : availableSurfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM || surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					InSwapchain.SurfaceFormat = surfaceFormat;
					break;
				}
			}

			YUKI_VERIFY(InSwapchain.SurfaceFormat.format != VK_FORMAT_UNDEFINED && InSwapchain.SurfaceFormat.colorSpace != VK_COLOR_SPACE_MAX_ENUM_KHR);
		}

		// Get viewport width and height
		{
			InSwapchain.Width = surfaceCapabilities.currentExtent.width;
			InSwapchain.Height = surfaceCapabilities.currentExtent.height;

			if (InSwapchain.Width == std::numeric_limits<uint32_t>::max())
			{
				const auto& windowAttributes = InSwapchain.Window->GetAttributes();
				InSwapchain.Width = Math::Clamp(windowAttributes.Width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
				InSwapchain.Height = Math::Clamp(windowAttributes.Height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
			}
		}

		{
			DynamicArray<VkPresentModeKHR> availablePresentModes;
			VulkanHelper::Enumerate(vkGetPhysicalDeviceSurfacePresentModesKHR, availablePresentModes, m_PhysicalDevice, InSwapchain.Surface);

			for (auto presentMode : availablePresentModes)
			{
				if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					InSwapchain.PresentMode = presentMode;
					break;
				}
			}
		}

		VkSwapchainCreateInfoKHR swapchainCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = InSwapchain.Surface,
			.minImageCount = Math::Min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount),
			.imageFormat = InSwapchain.SurfaceFormat.format,
			.imageColorSpace = InSwapchain.SurfaceFormat.colorSpace,
			.imageExtent = { InSwapchain.Width, InSwapchain.Height },
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &m_Queues.Get(GetGraphicsQueue()).FamilyIndex,
			.preTransform = surfaceCapabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = InSwapchain.PresentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = oldSwapchain,
		};
		vkCreateSwapchainKHR(m_LogicalDevice, &swapchainCreateInfo, nullptr, &InSwapchain.Swapchain);

		{
			if (!InSwapchain.Images.empty())
			{
				for (auto imageHandle : InSwapchain.Images)
				{
					auto& image = m_Images.Get(imageHandle);
					image.Image = VK_NULL_HANDLE;
					m_Images.Return(imageHandle);
				}
			}

			DynamicArray<VkImage> images;
			VulkanHelper::Enumerate(vkGetSwapchainImagesKHR, images, m_LogicalDevice, InSwapchain.Swapchain);
			InSwapchain.Images.resize(images.size());
			for (size_t i = 0; i < images.size(); i++)
			{
				auto[imageHandle, imageData] = m_Images.Acquire();
				imageData.Image = images[i];
				imageData.Width = InSwapchain.Width;
				imageData.Height = InSwapchain.Height;
				imageData.Format = InSwapchain.SurfaceFormat.format;
				imageData.AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
				imageData.Layout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageData.PipelineStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
				imageData.AccessFlags = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
				InSwapchain.Images[i] = imageHandle;
			}
		}

		{
			InSwapchain.ImageViews.resize(InSwapchain.Images.size());

			for (size_t i = 0; i < InSwapchain.Images.size(); i++)
				InSwapchain.ImageViews[i] = CreateImageView(InSwapchain.Images[i]);
		}

		InSwapchain.DepthImage = CreateImage(InSwapchain.Width, InSwapchain.Height, ImageFormat::Depth32SFloat, ImageUsage::DepthAttachment);

		while (InSwapchain.Semaphores.size() < InSwapchain.Images.size() * 2)
		{
			VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			VkSemaphore semaphore;
			vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &semaphore);
			InSwapchain.Semaphores.emplace_back(semaphore);
		}

		if (oldSwapchain != VK_NULL_HANDLE)
		{
			Destroy(oldDepthImage);
			vkDestroySwapchainKHR(m_LogicalDevice, oldSwapchain, nullptr);
		}
	}

	void VulkanRenderContext::Destroy(SwapchainHandle InSwapchain)
	{
		auto& swapchain = m_Swapchains.Get(InSwapchain);

		for (auto imageView : swapchain.ImageViews)
			Destroy(imageView);

		Destroy(swapchain.DepthImage);

		vkDestroySwapchainKHR(m_LogicalDevice, swapchain.Swapchain, nullptr);
		m_Swapchains.Return(InSwapchain);
	}

}
