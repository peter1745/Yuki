#include "VulkanSwapchain.hpp"
#include "VulkanHelper.hpp"
#include "VulkanPlatform.hpp"

#include "Math/Math.hpp"

namespace Yuki {

	/*bool VulkanSwapchain::AcquireNextImage(VulkanDevice* InDevice, VkSemaphore InSemaphore)
	{
		vkAcquireNextImageKHR(InDevice->GetLogicalDevice(), m_Swapchain, std::numeric_limits<uint64_t>::max(), InSemaphore, VK_NULL_HANDLE, &m_CurrentImage);
		return true;
	}*/

	VulkanSwapchain* VulkanSwapchain::Create(VulkanRenderContext* InContext, GenericWindow* InWindow)
	{
		auto* swapchain = new VulkanSwapchain();

		swapchain->m_Surface = VulkanPlatform::CreateSurface(InContext->GetInstance(), InWindow);

		auto surfaceCapabilities = InContext->QuerySurfaceCapabilities(swapchain->m_Surface);
		VkSurfaceFormatKHR selectedSurfaceFormat = { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
		{
			// Find a suitable surface format
			List<VkSurfaceFormatKHR> availableSurfaceFormats;
			VulkanHelper::Enumerate(vkGetPhysicalDeviceSurfaceFormatsKHR, availableSurfaceFormats, InContext->GetPhysicalDevice(), swapchain->m_Surface);
			for (const auto& surfaceFormat : availableSurfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM || surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					selectedSurfaceFormat = surfaceFormat;
					break;
				}
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
		{
			// Find a suitable present mode
			List<VkPresentModeKHR> availablePresentModes;
			VulkanHelper::Enumerate(vkGetPhysicalDeviceSurfacePresentModesKHR, availablePresentModes, InContext->GetPhysicalDevice(), swapchain->m_Surface);
			for (auto presentMode : availablePresentModes)
			{
				if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					selectedPresentMode = presentMode;
					break;
				}
			}
		}

		uint32_t queueFamilyIndex = InContext->GetGraphicsQueue()->GetFamilyIndex();
		VkSwapchainCreateInfoKHR swapchainInfo = {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = swapchain->m_Surface,
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
		YUKI_VERIFY(vkCreateSwapchainKHR(InContext->GetDevice(), &swapchainInfo, nullptr, &swapchain->m_Swapchain) == VK_SUCCESS);

		// Fetch the swapchain images and create image views
		{
			VulkanHelper::Enumerate(vkGetSwapchainImagesKHR, swapchain->m_Images, InContext->GetDevice(), swapchain->m_Swapchain);
			swapchain->m_ImageViews.reserve(swapchain->m_Images.size());
			for (auto image : swapchain->m_Images)
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
					    .layerCount = 1 }
				};

				VkImageView imageView;
				YUKI_VERIFY(vkCreateImageView(InContext->GetDevice(), &viewInfo, nullptr, &imageView) == VK_SUCCESS);
				swapchain->m_ImageViews.emplace_back(imageView);
			}
		}

		return swapchain;
	}

	void VulkanSwapchain::Destroy(VulkanRenderContext* InContext, VulkanSwapchain* InSwapchain)
	{
		for (auto imageView : InSwapchain->m_ImageViews)
			vkDestroyImageView(InContext->GetDevice(), imageView, nullptr);

		vkDestroySwapchainKHR(InContext->GetDevice(), InSwapchain->m_Swapchain, nullptr);
		vkDestroySurfaceKHR(InContext->GetInstance(), InSwapchain->m_Surface, nullptr);

		delete InSwapchain;
	}

}
