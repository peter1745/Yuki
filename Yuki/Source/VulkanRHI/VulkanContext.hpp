#pragma once

#include "Engine/RHI/Context.hpp"

#include "VulkanInclude.hpp"
#include "VulkanRenderDevice.hpp"

namespace Yuki::RHI {

	class VulkanContext final : public Context
	{
	public:
		VulkanContext(ContextConfig InConfig);
		~VulkanContext() noexcept;

	public:
		RenderDevice& GetRenderDevice() const override { return m_RenderDevice; }

	private:
		void CreateInstance(VkDebugUtilsMessengerCreateInfoEXT& InDebugUtilsMessengerInfo);

	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessengerHandle = VK_NULL_HANDLE;

		Unique<VulkanRenderDevice> m_RenderDevice = nullptr;

		bool m_EnableValidation = true;
	};

}
