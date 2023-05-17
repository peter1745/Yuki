#include "VulkanSwapchain.hpp"
#include "VulkanHelper.hpp"
#include "VulkanPlatform.hpp"

#include "Math/Math.hpp"

namespace Yuki {

	VulkanSwapchain::VulkanSwapchain(GenericWindow* InWindow, VkInstance InInstance, VulkanDevice* InDevice)
	{
		m_Surface = VulkanPlatform::CreateSurface(InInstance, InWindow);

		auto surfaceCapabilities = InDevice->QuerySurfaceCapabilities(m_Surface);

		VkSurfaceFormatKHR selectedSurfaceFormat = { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
		List<VkSurfaceFormatKHR> availableSurfaceFormats;
		VulkanHelper::Enumerate(vkGetPhysicalDeviceSurfaceFormatsKHR, availableSurfaceFormats, InDevice->GetPhysicalDevice(), m_Surface);
		for (const auto& surfaceFormat : availableSurfaceFormats)
		{
			if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM || surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				selectedSurfaceFormat = surfaceFormat;
				break;
			}
		}

		VkExtent2D imageExtent = surfaceCapabilities.currentExtent;
		if (imageExtent.width == std::numeric_limits<uint32_t>::max())
		{
			const auto& windowAttributes = InWindow->GetAttributes();
			imageExtent.width = Math::Clamp(windowAttributes.Width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			imageExtent.height = Math::Clamp(windowAttributes.Height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}

		VkPresentModeKHR selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		List<VkPresentModeKHR> availablePresentModes;
		VulkanHelper::Enumerate(vkGetPhysicalDeviceSurfacePresentModesKHR, availablePresentModes, InDevice->GetPhysicalDevice(), m_Surface);
		for (auto presentMode : availablePresentModes)
		{
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				selectedPresentMode = presentMode;
				break;
			}
		}

		uint32_t queueFamilyIndex = InDevice->GetQueueFamilyIndex();
		VkSwapchainCreateInfoKHR swapchainInfo = {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = m_Surface,
			.minImageCount = Math::Min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount),
			.imageFormat = selectedSurfaceFormat.format,
			.imageColorSpace = selectedSurfaceFormat.colorSpace,
			.imageExtent = imageExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &queueFamilyIndex,
			.preTransform = surfaceCapabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = selectedPresentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE,
		};
		YUKI_VERIFY(vkCreateSwapchainKHR(InDevice->GetLogicalDevice(), &swapchainInfo, nullptr, &m_Swapchain) == VK_SUCCESS);

		VulkanHelper::Enumerate(vkGetSwapchainImagesKHR, m_Images, InDevice->GetLogicalDevice(), m_Swapchain);
		m_ImageViews.reserve(m_Images.size());
		for (auto image : m_Images)
		{
			VkImageViewCreateInfo viewInfo = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = image,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = selectedSurfaceFormat.format,
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			VkImageView imageView;
			YUKI_VERIFY(vkCreateImageView(InDevice->GetLogicalDevice(), &viewInfo, nullptr, &imageView) == VK_SUCCESS);
			m_ImageViews.emplace_back(imageView);
		}
	}

	bool VulkanSwapchain::AcquireNextImage(VulkanDevice* InDevice, VkSemaphore InSemaphore)
	{
		vkAcquireNextImageKHR(InDevice->GetLogicalDevice(), m_Swapchain, std::numeric_limits<uint64_t>::max(), InSemaphore, VK_NULL_HANDLE, &m_CurrentImage);
		return true;
	}

}
