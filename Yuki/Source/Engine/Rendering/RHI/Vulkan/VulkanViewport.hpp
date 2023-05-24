#pragma once

#include "Rendering/RHI/Viewport.hpp"

#include "VulkanRenderContext.hpp"
#include "VulkanSwapchain.hpp"

namespace Yuki {

	class VulkanViewport : public Viewport
	{
	public:
		VulkanViewport(VulkanRenderContext* InContext, GenericWindow* InWindow);
		~VulkanViewport();

		GenericWindow* GetWindow() const override { return m_Window; }

		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }

		Swapchain* GetSwapchain() const override { return m_Swapchain; }

		void AcquireNextImage() override;
		void RecreateSwapchain() override;

	private:
		VulkanRenderContext* m_Context = nullptr;
		GenericWindow* m_Window = nullptr;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		VulkanSwapchain* m_Swapchain = nullptr;

		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkSurfaceFormatKHR m_SurfaceFormat = { VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_MAX_ENUM_KHR };
		VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;
	};

}
