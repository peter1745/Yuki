#include "VulkanRenderContext.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	VulkanRenderContext::VulkanRenderContext(GenericWindow* InWindow)
	    : m_Window(InWindow), m_Platform(VulkanPlatform::New())
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
			enabledLayers.EmplaceBack("VK_LAYER_KHRONOS_validation");
			enabledInstanceExtensions.EmplaceBack(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		enabledInstanceExtensions.EmplaceBack(VK_KHR_SURFACE_EXTENSION_NAME);
		m_Platform->GetRequiredInstanceExtensions(enabledInstanceExtensions);

		VkApplicationInfo appInfo = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.apiVersion = VK_API_VERSION_1_3
		};

		VkInstanceCreateInfo instanceInfo = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = uint32_t(enabledLayers.Count()),
			.ppEnabledLayerNames = enabledLayers.Data(),
			.enabledExtensionCount = uint32_t(enabledInstanceExtensions.Count()),
			.ppEnabledExtensionNames = enabledInstanceExtensions.Data(),
		};

		YUKI_VERIFY(vkCreateInstance(&instanceInfo, nullptr, &m_Instance) == VK_SUCCESS);

		volkLoadInstance(m_Instance);

		m_Surface = m_Platform->CreateSurface(m_Instance, m_Window);

		SelectSuitablePhysicalDevice();

		m_Device->CreateLogicalDevice(m_Surface, enabledLayers);
	}

	void VulkanRenderContext::Destroy()
	{
		m_Device->WaitIdle();
		m_Device->Destroy();

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
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
		const auto& availableDevices = m_Platform->QueryAvailableDevices(m_Instance);

		size_t selectedDeviceIndex = 0;
		uint32_t highestDeviceScore = 0;

		for (size_t i = 0; i < availableDevices.Count(); i++)
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

	Unique<RenderContext> RenderContext::New(GenericWindow* InWindow) { return Unique<VulkanRenderContext>::Create(InWindow); }

}
