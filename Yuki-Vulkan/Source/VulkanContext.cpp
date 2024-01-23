#include "VulkanRHI.hpp"

#include <array>

namespace Yuki {

	static constexpr std::string_view s_ValidationLayerName = "VK_LAYER_KHRONOS_validation";

	static bool IsValidationSupported()
	{
		std::vector<VkLayerProperties> supportedLayers;
		Vulkan::Enumerate(vkEnumerateInstanceLayerProperties, supportedLayers);

		for (const auto& layer : supportedLayers)
		{
			std::string_view name = layer.layerName;

			if (name == s_ValidationLayerName)
			{
				return true;
			}
		}

		return false;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
	{
		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			WriteLine("{}", callbackData->pMessage);
		}
		else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			WriteLine("{}", LogLevel::Warn, callbackData->pMessage);
		}
		else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			WriteLine("{}", LogLevel::Error, callbackData->pMessage);
			YukiAssert(false);
		}

		return VK_FALSE;
	}

	static std::vector<Queue> RequestQueues(RHIContext context, VkQueueFlagBits queueType, uint32_t count)
	{
		std::vector<VkQueueFamilyProperties> queueFamilies;
		Vulkan::Enumerate(vkGetPhysicalDeviceQueueFamilyProperties, queueFamilies, context->PhysicalDevice);

		uint32_t familyIndex = 0;
		uint32_t bestFamilyIndex = 0;
		uint32_t bestScore = 0;

		for (const auto& queueFamily : queueFamilies)
		{
			if ((queueFamily.queueFlags & queueType) != static_cast<uint32_t>(queueType))
			{
				continue;
			}

			uint32_t score = std::popcount<uint32_t>(queueFamily.queueFlags & ~static_cast<uint32_t>(queueType));

			if (score > bestScore)
			{
				bestScore = score;
				bestFamilyIndex = familyIndex;
			}

			familyIndex++;
		}

		const auto& bestFamily = queueFamilies[bestFamilyIndex];

		std::vector<Queue> result;
		result.reserve(count);

		for (uint32_t i = 0; i < count; i++)
		{
			if (i >= bestFamily.queueCount)
			{
				break;
			}

			auto queue = new Queue::Impl();
			queue->Context = context;
			queue->Family = bestFamilyIndex;
			queue->Index = i;
			queue->Flags = bestFamily.queueFlags;
			queue->Priority = 1.0f;
			result.push_back({ queue });
		}

		return result;
	}

	RHIContext RHIContext::Create()
	{
		auto* impl = new Impl();

		// Initialize volk
		Vulkan::CheckResult(volkInitialize());

		// Initialize Vulkan
		std::vector<const char*> instanceLayers;
		std::vector<const char*> instanceExtensions;

		instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(YUKI_PLATFORM_WINDOWS)
		instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

		impl->ValidationEnabled = IsValidationSupported();

		if (impl->ValidationEnabled)
		{
			instanceLayers.push_back(s_ValidationLayerName.data());
			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			debugMessengerInfo.messageSeverity =
				//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

			debugMessengerInfo.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

			debugMessengerInfo.pfnUserCallback = DebugMessengerCallback;
		}

		VkApplicationInfo appInfo =
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.apiVersion = VK_API_VERSION_1_3
		};

		VkInstanceCreateInfo instanceInfo =
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = &debugMessengerInfo,
			.flags = 0,
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
			.ppEnabledLayerNames = instanceLayers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
			.ppEnabledExtensionNames = instanceExtensions.data(),
		};

		Vulkan::CheckResult(vkCreateInstance(&instanceInfo, nullptr, &impl->Instance));

		// Load instance-related function pointers
		volkLoadInstanceOnly(impl->Instance);

		// Fully setup the debug messenger if validation is supported
		if (impl->ValidationEnabled)
		{
			Vulkan::CheckResult(vkCreateDebugUtilsMessengerEXT(
				impl->Instance,
				&debugMessengerInfo,
				nullptr,
				&impl->DebugMessenger
			));
		}

		// Find a suitable physical device
		std::vector<VkPhysicalDevice> availablePhysicalDevices;
		Vulkan::Enumerate(vkEnumeratePhysicalDevices, availablePhysicalDevices, impl->Instance);
		for (auto physicalDevice : availablePhysicalDevices)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

			if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				continue;

			impl->PhysicalDevice = physicalDevice;
			break;
		}

		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(impl->PhysicalDevice, &physicalDeviceProperties);
		WriteLine("GPU: {}", physicalDeviceProperties.deviceName);

		// Create a logical device
		impl->Queues.append_range(RequestQueues({ impl }, VK_QUEUE_GRAPHICS_BIT, 1));

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		for (auto queue : impl->Queues)
		{
			VkDeviceQueueCreateInfo queueInfo =
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = queue->Family,
				.queueCount = 1,
				.pQueuePriorities = &queue->Priority,
			};

			queueCreateInfos.push_back(std::move(queueInfo));
		}

		VkDeviceCreateInfo deviceInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos = queueCreateInfos.data(),
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = nullptr,
			.enabledExtensionCount = 0,
			.ppEnabledExtensionNames = nullptr,
			.pEnabledFeatures = nullptr,
		};
		Vulkan::CheckResult(vkCreateDevice(impl->PhysicalDevice, &deviceInfo, nullptr, &impl->Device));

		volkLoadDevice(impl->Device);

		impl->Allocator = VulkanMemoryAllocator::Create(impl->Instance, impl->PhysicalDevice, impl->Device);

		return { impl };
	}

	void RHIContext::Destroy()
	{
		m_Impl->Allocator.Destroy();

		vkDestroyDevice(m_Impl->Device, nullptr);

		if (m_Impl->ValidationEnabled)
		{
			vkDestroyDebugUtilsMessengerEXT(m_Impl->Instance, m_Impl->DebugMessenger, nullptr);
		}

		vkDestroyInstance(m_Impl->Instance, nullptr);
	}

}
