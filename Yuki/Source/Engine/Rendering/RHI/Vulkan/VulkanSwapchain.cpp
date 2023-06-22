#include "VulkanSwapchain.hpp"
#include "VulkanHelper.hpp"
#include "VulkanPlatform.hpp"

#include "Math/Math.hpp"

namespace Yuki {

	VulkanSwapchain::VulkanSwapchain(VulkanRenderContext* InContext, const VulkanSwapchainInfo& InSwapchainInfo)
		: m_Context(InContext)
	{
		Create(InSwapchainInfo);
	}

	void VulkanSwapchain::Destroy()
	{
		for (auto semaphore : m_Semaphores)
			vkDestroySemaphore(m_Context->GetDevice(), semaphore, nullptr);
		m_Semaphores.clear();
		m_SemaphoreIndex = 0;

		m_ImageViews.clear();
		m_Images.clear();

		vkDestroySwapchainKHR(m_Context->GetDevice(), m_Swapchain, nullptr);
	}

	void VulkanSwapchain::Create(const VulkanSwapchainInfo& InSwapchainInfo)
	{
		uint32_t queueFamilyIndex = static_cast<VulkanQueue*>(m_Context->GetGraphicsQueue())->GetFamilyIndex();
		VkSwapchainCreateInfoKHR swapchainInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = InSwapchainInfo.Surface,
			.minImageCount = InSwapchainInfo.MinImageCount,
			.imageFormat = InSwapchainInfo.SurfaceFormat.format,
			.imageColorSpace = InSwapchainInfo.SurfaceFormat.colorSpace,
			.imageExtent = InSwapchainInfo.ImageExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &queueFamilyIndex,
			.preTransform = InSwapchainInfo.PreTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = InSwapchainInfo.PresentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE,
		};
		YUKI_VERIFY(vkCreateSwapchainKHR(m_Context->GetDevice(), &swapchainInfo, nullptr, &m_Swapchain) == VK_SUCCESS);

		// Fetch the swapchain images and create image views
		{
			List<VkImage> swapchainImages;
			VulkanHelper::Enumerate(vkGetSwapchainImagesKHR, swapchainImages, m_Context->GetDevice(), m_Swapchain);
			m_Images.reserve(swapchainImages.size());
			for (auto& image : swapchainImages)
				m_Images.emplace_back(Unique(new VulkanImage2D(m_Context, InSwapchainInfo.ImageExtent.width, InSwapchainInfo.ImageExtent.height, VulkanHelper::VkFormatToImageFormat(InSwapchainInfo.SurfaceFormat.format), image)));

			m_ImageViews.reserve(m_Images.size());
			for (auto& image : m_Images)
				m_ImageViews.emplace_back(m_Context->CreateImageView2D(image));
		}

		m_DepthImage = m_Context->CreateImage2D(InSwapchainInfo.ImageExtent.width, InSwapchainInfo.ImageExtent.height, ImageFormat::Depth32SFloat, ImageUsage::DepthAttachment);

		// Create binary semaphores
		{
			m_Semaphores.resize(m_Images.size() * 2);

			VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

			for (size_t i = 0; i < m_Semaphores.size(); i++)
				vkCreateSemaphore(m_Context->GetDevice(), &semaphoreInfo, nullptr, &m_Semaphores[i]);
		}
	}

	void VulkanSwapchain::Recreate(const VulkanSwapchainInfo& InSwapchainInfo)
	{
		Destroy();
		Create(InSwapchainInfo);
	}

	VkResult VulkanSwapchain::AcquireNextImage()
	{
		VkAcquireNextImageInfoKHR acquireImageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
			.swapchain = m_Swapchain,
			.timeout = UINT64_MAX,
			.semaphore = m_Semaphores[m_SemaphoreIndex],
			.fence = VK_NULL_HANDLE,
			.deviceMask = 1
		};

		return vkAcquireNextImage2KHR(m_Context->GetDevice(), &acquireImageInfo, &m_CurrentImageIndex);
	}

}
