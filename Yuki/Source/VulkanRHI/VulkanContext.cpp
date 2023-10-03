#include "VulkanContext.hpp"
#include "VulkanUtils.hpp"
#include "VulkanRenderFeatures.hpp"
#include "VulkanPlatformUtils.hpp"

namespace Yuki::RHI {

	Unique<Context> Context::New(ContextConfig InConfig)
	{
		return new VulkanContext(InConfig);
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT InSeverity, VkDebugUtilsMessageTypeFlagsEXT InType, const VkDebugUtilsMessengerCallbackDataEXT* InCallbackData, void* InUserData)
	{
		if (InSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			Logging::Info("{}", InCallbackData->pMessage);
		}
		else if (InSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			Logging::Warn("{}", InCallbackData->pMessage);
		}
		else if (InSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			Logging::Error("{}", InCallbackData->pMessage);
			YUKI_VERIFY(false);
		}

		return VK_FALSE;
	}

	VulkanContext::VulkanContext(ContextConfig InConfig)
	{
		volkInitialize();

		// Check if validation is available, disable if it's not
		{
			bool ValidationSupported = false;

			DynamicArray<VkLayerProperties> Layers;
			Vulkan::Enumerate(vkEnumerateInstanceLayerProperties, Layers);
			for (const auto& Layer : Layers)
			{
				if (std::string_view(Layer.layerName) == "VK_LAYER_KHRONOS_validation")
				{
					ValidationSupported = true;
					break;
				}
			}

			m_EnableValidation = m_EnableValidation && ValidationSupported;
		}

		VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
		};

		CreateInstance(MessengerCreateInfo);

		if (m_EnableValidation)
			vkCreateDebugUtilsMessengerEXT(m_Instance, &MessengerCreateInfo, nullptr, &m_DebugMessengerHandle);

		m_RenderDevice = Unique<VulkanRenderDevice>(new VulkanRenderDevice(m_Instance, InConfig.RequestedFeatures));
	}

	VulkanContext::~VulkanContext() noexcept
	{
		if (m_DebugMessengerHandle)
			vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessengerHandle, nullptr);

		vkDestroyInstance(m_Instance, nullptr);
	}

	void VulkanContext::CreateInstance(VkDebugUtilsMessengerCreateInfoEXT& InDebugUtilsMessengerInfo)
	{
		DynamicArray<const char*> EnabledInstanceLayers;
		DynamicArray<const char*> EnabledInstanceExtensions = {
			VK_KHR_SURFACE_EXTENSION_NAME,
		};

		Vulkan::AddPlatformInstanceExtensions(EnabledInstanceExtensions);

		if (m_EnableValidation)
		{
			EnabledInstanceLayers.emplace_back("VK_LAYER_KHRONOS_validation");
			EnabledInstanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			constexpr uint32_t MessageSeverities =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT	|
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			constexpr uint32_t MessageTypes =
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

			InDebugUtilsMessengerInfo.messageSeverity = MessageSeverities;
			InDebugUtilsMessengerInfo.messageType = MessageTypes;
			InDebugUtilsMessengerInfo.pfnUserCallback = DebugMessengerCallback;
		}

		VkApplicationInfo AppInfo =
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.apiVersion = VK_API_VERSION_1_3
		};

		VkInstanceCreateInfo InstanceInfo =
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = &InDebugUtilsMessengerInfo,
			.flags = 0,
			.pApplicationInfo = &AppInfo,
			.enabledLayerCount = static_cast<uint32_t>(EnabledInstanceLayers.size()),
			.ppEnabledLayerNames = EnabledInstanceLayers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(EnabledInstanceExtensions.size()),
			.ppEnabledExtensionNames = EnabledInstanceExtensions.data(),
		};

		vkCreateInstance(&InstanceInfo, nullptr, &m_Instance);

		volkLoadInstanceOnly(m_Instance);
	}

}
