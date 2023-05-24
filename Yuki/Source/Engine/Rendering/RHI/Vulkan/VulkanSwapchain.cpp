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

	void VulkanSwapchain::BeginRendering(CommandBuffer InCmdBuffer)
	{
		VulkanImageTransition imageTransition = {
			.DstPipelineStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.DstAccessFlags = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.DstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		m_Images[m_CurrentImage]->Transition(InCmdBuffer.As<VkCommandBuffer>(), imageTransition);

		VkClearValue clearColor = {};
		clearColor.color.float32[0] = 1.0f;
		clearColor.color.float32[1] = 0.0f;
		clearColor.color.float32[2] = 0.0f;
		clearColor.color.float32[3] = 1.0f;

		VkRenderingAttachmentInfo colorAttachmentInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.imageView = m_ImageViews[m_CurrentImage]->GetVkImageView(),
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = clearColor,
		};

		VkRenderingInfo renderingInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea = {
			    .offset = { 0, 0 },
			    .extent = { m_Images[m_CurrentImage]->GetWidth(), m_Images[m_CurrentImage]->GetHeight() },
			},
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo
		};

		vkCmdBeginRendering(InCmdBuffer.As<VkCommandBuffer>(), &renderingInfo);
	}

	void VulkanSwapchain::EndRendering(CommandBuffer InCmdBuffer)
	{
		vkCmdEndRendering(InCmdBuffer.As<VkCommandBuffer>());

		VulkanImageTransition imageTransition = {
			.DstPipelineStage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
			.DstAccessFlags = 0,
			.DstImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		m_Images[m_CurrentImage]->Transition(InCmdBuffer.As<VkCommandBuffer>(), imageTransition);
	}

	void VulkanSwapchain::Create(const VulkanSwapchainInfo& InSwapchainInfo)
	{
		uint32_t queueFamilyIndex = static_cast<VulkanQueue*>(m_Context->GetGraphicsQueue())->GetFamilyIndex();
		VkSwapchainCreateInfoKHR swapchainInfo = {
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
			for (auto image : swapchainImages)
				m_Images.emplace_back(VulkanImage2D::Create(m_Context, InSwapchainInfo.ImageExtent.width, InSwapchainInfo.ImageExtent.height, VulkanHelper::VkFormatToImageFormat(InSwapchainInfo.SurfaceFormat.format), image));

			m_ImageViews.reserve(m_Images.size());
			for (auto image : m_Images)
				m_ImageViews.emplace_back(static_cast<VulkanImageView2D*>(m_Context->CreateImageView2D(image)));
		}

		// Create binary semaphores
		{
			m_Semaphores.resize(m_Images.size() * 2);

			VkSemaphoreCreateInfo semaphoreInfo = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

			for (size_t i = 0; i < m_Semaphores.size(); i++)
				vkCreateSemaphore(m_Context->GetDevice(), &semaphoreInfo, nullptr, &m_Semaphores[i]);
		}
	}

	void VulkanSwapchain::Recreate(const VulkanSwapchainInfo& InSwapchainInfo)
	{
		// Destroy old resources
		{
			for (auto semaphore : m_Semaphores)
			{
				LogInfo("Destroying semaphore: {}", (void*)semaphore);
				vkDestroySemaphore(m_Context->GetDevice(), semaphore, nullptr);
			}
			m_Semaphores.clear();
			m_SemaphoreIndex = 0;

			for (auto imageView : m_ImageViews)
				m_Context->DestroyImageView2D(imageView);
			m_ImageViews.clear();

			m_Images.clear();

			m_CurrentImage = 0;

			vkDestroySwapchainKHR(m_Context->GetDevice(), m_Swapchain, nullptr);
		}

		Create(InSwapchainInfo);
	}

	VkResult VulkanSwapchain::AcquireNextImage()
	{
		LogInfo("Acquiring with semaphore {}", (void*)m_Semaphores[m_SemaphoreIndex]);

		VkAcquireNextImageInfoKHR acquireImageInfo = {
			.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
			.swapchain = m_Swapchain,
			.timeout = UINT64_MAX,
			.semaphore = m_Semaphores[m_SemaphoreIndex],
			.fence = VK_NULL_HANDLE,
			.deviceMask = 1
		};

		return vkAcquireNextImage2KHR(m_Context->GetDevice(), &acquireImageInfo, &m_CurrentImage);
	}

}
