#include "VulkanViewport.hpp"
#include "VulkanPlatform.hpp"
#include "VulkanHelper.hpp"

#include "Math/Math.hpp"

namespace Yuki {

	VulkanViewport::VulkanViewport(VulkanRenderContext* InContext, GenericWindow* InWindow)
		: m_Context(InContext), m_Window(InWindow)
	{
		m_Surface = VulkanPlatform::CreateSurface(InContext->GetInstance(), InWindow);

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(InContext->GetPhysicalDevice(), m_Surface, &surfaceCapabilities);

		// Find suitable surface format
		{
			List<VkSurfaceFormatKHR> availableSurfaceFormats;
			VulkanHelper::Enumerate(vkGetPhysicalDeviceSurfaceFormatsKHR, availableSurfaceFormats, InContext->GetPhysicalDevice(), m_Surface);

			for (const auto& surfaceFormat : availableSurfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM || surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					m_SurfaceFormat = surfaceFormat;
					break;
				}
			}

			YUKI_VERIFY(m_SurfaceFormat.format != VK_FORMAT_UNDEFINED && m_SurfaceFormat.colorSpace != VK_COLOR_SPACE_MAX_ENUM_KHR);
		}

		// Get viewport width and height
		{
			m_Width = surfaceCapabilities.currentExtent.width;
			m_Height = surfaceCapabilities.currentExtent.height;

			if (m_Width == std::numeric_limits<uint32_t>::max())
			{
				const auto& windowAttributes = InWindow->GetAttributes();
				m_Width = Math::Clamp(windowAttributes.Width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
				m_Height = Math::Clamp(windowAttributes.Height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
			}
		}

		// Find suitable present mode
		{
			// Use FIFO in case MAILBOX isn't available (FIFO is the only present mode that is guaranteed to be supported)
			m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;

			List<VkPresentModeKHR> availablePresentModes;
			VulkanHelper::Enumerate(vkGetPhysicalDeviceSurfacePresentModesKHR, availablePresentModes, InContext->GetPhysicalDevice(), m_Surface);

			for (auto presentMode : availablePresentModes)
			{
				if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					m_PresentMode = presentMode;
					break;
				}
			}
		}

		VulkanSwapchainInfo swapchainInfo = {
			.Surface = m_Surface,
			.SurfaceFormat = m_SurfaceFormat,
			.PresentMode = m_PresentMode,
			.MinImageCount = Math::Min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount),
			.ImageExtent = { m_Width, m_Height },
			.PreTransform = surfaceCapabilities.currentTransform,
		};

		m_Swapchain = new VulkanSwapchain(InContext, swapchainInfo);
	}

	VulkanViewport::~VulkanViewport()
	{
		m_Context->GetGraphicsQueue()->WaitIdle();

		m_Swapchain->Destroy();
		delete m_Swapchain;

		vkDestroySurfaceKHR(m_Context->GetInstance(), m_Surface, nullptr);
	}

	void VulkanViewport::SetViewportAndScissor(CommandBuffer* InCmdBuffer)
	{
		VkViewport viewport =
		{
			.x = 0.0f,
			.y = 0.0f,
			.width = float(m_Width),
			.height = float(m_Height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};
		vkCmdSetViewport(InCmdBuffer->As<VkCommandBuffer>(), 0, 1, &viewport);

		VkRect2D scissor =
		{
			.offset =
			{
				.x = 0,
				.y = 0,
			},
			.extent =
			{
				.width = m_Width,
				.height = m_Height,
			},
		};
		vkCmdSetScissor(InCmdBuffer->As<VkCommandBuffer>(), 0, 1, &scissor);
	}

	void VulkanViewport::AcquireNextImage()
	{
		LogInfo("Acquiring Next Image (Viewport: {}, Swapchain: {}, VkSwapchain: {})", (void*)this, (void*)m_Swapchain, (void*)m_Swapchain->GetVkSwapchain());

		VkResult acquireResult = m_Swapchain->AcquireNextImage();

		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapchain();
			YUKI_VERIFY(m_Swapchain->AcquireNextImage() == VK_SUCCESS);
		}
	}

	void VulkanViewport::RecreateSwapchain()
	{
		m_Context->WaitDeviceIdle();

		LogInfo("Recreating Swapchain {}", (void*)m_Swapchain);

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Context->GetPhysicalDevice(), m_Surface, &surfaceCapabilities);

		// Update viewport width and height
		{
			m_Width = surfaceCapabilities.currentExtent.width;
			m_Height = surfaceCapabilities.currentExtent.height;

			if (m_Width == std::numeric_limits<uint32_t>::max())
			{
				const auto& windowAttributes = m_Window->GetAttributes();
				m_Width = Math::Clamp(windowAttributes.Width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
				m_Height = Math::Clamp(windowAttributes.Height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
			}
		}

		VulkanSwapchainInfo swapchainInfo = {
			.Surface = m_Surface,
			.SurfaceFormat = m_SurfaceFormat,
			.PresentMode = m_PresentMode,
			.MinImageCount = Math::Min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount),
			.ImageExtent = {m_Width, m_Height},
			.PreTransform = surfaceCapabilities.currentTransform,
		};

		m_Swapchain->Recreate(swapchainInfo);

		LogInfo("Recreated Swapchain {} (VkSwapchain: {})", (void*)m_Swapchain, (void*)m_Swapchain->GetVkSwapchain());
	}

}
