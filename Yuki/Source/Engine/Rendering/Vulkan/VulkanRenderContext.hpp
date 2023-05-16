#pragma once

#include "Rendering/RenderContext.hpp"

#include "Vulkan.hpp"
#include "VulkanPlatform.hpp"
#include "VulkanDevice.hpp"

namespace Yuki {

	class VulkanRenderContext : public RenderContext
	{
	public:
		VulkanRenderContext(GenericWindow* InWindow);
		~VulkanRenderContext() = default;

		void Initialize() override;
		void Destroy() override;

	private:
		bool HasValidationLayerSupport() const;
		void SelectSuitablePhysicalDevice();

	private:
		GenericWindow* m_Window = nullptr;
		Unique<VulkanPlatform> m_Platform = nullptr;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VulkanDevice* m_Device = nullptr;
		VkQueue m_Queue = VK_NULL_HANDLE;
	};

}
