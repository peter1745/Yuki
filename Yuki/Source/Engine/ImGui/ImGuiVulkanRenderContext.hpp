#pragma once

#include "ImGui/ImGuiRenderContext.hpp"

#include "Rendering/RenderResources.hpp"

#include "../Rendering/Vulkan/VulkanInclude.hpp"

namespace Yuki {

	struct VulkanSwapchain;

	class ImGuiVulkanRenderContext : public ImGuiRenderContext
	{
	public:
		ImGuiVulkanRenderContext(SwapchainHandle InSwapchain, RenderContext* InContext);

		void NewFrame(CommandListHandle InCommandList) const override;
		void EndFrame(CommandListHandle InCommandList) override;

	private:
		RenderContext* m_Context = nullptr;
		VulkanSwapchain* m_Swapchain = nullptr;
		DescriptorPool m_DescriptorPool{};
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;

		uint32_t m_LastWidth = 0;
		uint32_t m_LastHeight = 0;
	};

}
