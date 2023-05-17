#include "VulkanRenderContext.hpp"
#include "VulkanHelper.hpp"
#include "VulkanSwapchain.hpp"

namespace Yuki {

	VulkanRenderContext::VulkanRenderContext()
	{
		YUKI_VERIFY(volkInitialize() == VK_SUCCESS);
	}

	void VulkanRenderContext::Initialize()
	{
		List<const char*> enabledLayers;
		List<const char*> enabledInstanceExtensions;

		bool enableValidationLayers = HasValidationLayerSupport() && true;

		if (enableValidationLayers)
		{
			enabledLayers.emplace_back("VK_LAYER_KHRONOS_validation");
			enabledInstanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		enabledInstanceExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
		VulkanPlatform::GetRequiredInstanceExtensions(enabledInstanceExtensions);

		VkApplicationInfo appInfo = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.apiVersion = VK_API_VERSION_1_3
		};

		VkInstanceCreateInfo instanceInfo = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = uint32_t(enabledLayers.size()),
			.ppEnabledLayerNames = enabledLayers.data(),
			.enabledExtensionCount = uint32_t(enabledInstanceExtensions.size()),
			.ppEnabledExtensionNames = enabledInstanceExtensions.data(),
		};

		YUKI_VERIFY(vkCreateInstance(&instanceInfo, nullptr, &m_Instance) == VK_SUCCESS);

		volkLoadInstance(m_Instance);

		SelectSuitablePhysicalDevice();

		m_Device->CreateLogicalDevice(enabledLayers);
	}

	void VulkanRenderContext::Destroy()
	{
		m_Device->WaitIdle();
		m_Device->Destroy();

		vkDestroyInstance(m_Instance, nullptr);
	}

	Unique<Swapchain> VulkanRenderContext::CreateSwapchain(GenericWindow* InWindow) const
	{
		return Unique<VulkanSwapchain>::Create(InWindow, m_Instance, m_Device);
	}

	bool VulkanRenderContext::HasValidationLayerSupport() const
	{
		bool supportsValidationLayers = false;

		List<VkLayerProperties> availableLayers;
		VulkanHelper::Enumerate(vkEnumerateInstanceLayerProperties, availableLayers);
		for (const auto& layerInfo : availableLayers)
		{
			std::string_view layerName = layerInfo.layerName;

			if (layerName == "VK_LAYER_KHRONOS_validation")
			{
				supportsValidationLayers = true;
				break;
			}
		}

		return supportsValidationLayers;
	}

	void VulkanRenderContext::SelectSuitablePhysicalDevice()
	{
		const auto& availableDevices = VulkanPlatform::QueryAvailableDevices(m_Instance);

		size_t selectedDeviceIndex = 0;
		uint32_t highestDeviceScore = 0;

		for (size_t i = 0; i < availableDevices.size(); i++)
		{
			const auto& device = availableDevices[i];

			if (device->GetDeviceScore() > highestDeviceScore)
			{
				highestDeviceScore = device->GetDeviceScore();
				selectedDeviceIndex = i;
			}
		}

		const auto& selectedDevice = availableDevices[selectedDeviceIndex];
		LogInfo("Selected GPU:");
		LogInfo("\tName: {}", selectedDevice->GetPhysicalDeviceProperties().deviceName);
		LogInfo("\tDriver Version: {}", selectedDevice->GetPhysicalDeviceProperties().driverVersion);
		LogInfo("\tScore: {}", selectedDevice->GetDeviceScore());

		m_Device = selectedDevice.GetPtr();
	}

}
