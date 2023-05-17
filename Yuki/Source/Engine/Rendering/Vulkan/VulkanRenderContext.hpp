#pragma once

#include "Rendering/RenderContext.hpp"

#include "Vulkan.hpp"
#include "VulkanPlatform.hpp"
#include "VulkanDevice.hpp"

namespace Yuki {

	class VulkanRenderContext : public RenderContext
	{
	public:
		VulkanRenderContext();
		~VulkanRenderContext() = default;

		void Initialize() override;
		void Destroy() override;

		Unique<Swapchain> CreateSwapchain(GenericWindow* InWindow) const override;

	private:
		bool HasValidationLayerSupport() const;
		void SelectSuitablePhysicalDevice();

	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
		VulkanDevice* m_Device = nullptr;
		VkQueue m_Queue = VK_NULL_HANDLE;
	};

}
