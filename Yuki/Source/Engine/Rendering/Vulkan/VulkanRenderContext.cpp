#include "VulkanRenderContext.hpp"

#include "VulkanPlatform.hpp"
#include "VulkanHelper.hpp"

#include <glslang/Public/ShaderLang.h>

namespace Yuki {

	static constexpr uint32_t s_MessageSeverities = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	static constexpr uint32_t s_MessageTypes = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	bool HasValidationLayerSupport()
	{
		bool supportsValidationLayers = false;

		DynamicArray<VkLayerProperties> availableLayers;
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

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT InSeverity, VkDebugUtilsMessageTypeFlagsEXT InType, const VkDebugUtilsMessengerCallbackDataEXT* InCallbackData, void* InUserData)
	{
		if (InSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		{
			LogDebug("Vulkan: {}", InCallbackData->pMessage);
		}
		else if (InSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			LogInfo("Vulkan: {}", InCallbackData->pMessage);
		}
		else if (InSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			LogWarn("Vulkan: {}", InCallbackData->pMessage);
		}
		else if (InSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			LogError("Vulkan: {}", InCallbackData->pMessage);
			YUKI_VERIFY(false);
		}

		return VK_FALSE;
	}

	VulkanRenderContext::VulkanRenderContext()
	{
		YUKI_VERIFY(volkInitialize() == VK_SUCCESS);

		DynamicArray<const char*> enabledLayers;
		DynamicArray<const char*> enabledInstanceExtensions;

		bool enableValidationLayers = HasValidationLayerSupport() && s_CurrentConfig != Configuration::Release;

		if (enableValidationLayers)
		{
			enabledLayers.emplace_back("VK_LAYER_KHRONOS_validation");
			enabledInstanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		enabledInstanceExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

		VulkanPlatform::GetRequiredInstanceExtensions(enabledInstanceExtensions);

		VkApplicationInfo appInfo =
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.apiVersion = VK_API_VERSION_1_3
		};

		VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity = s_MessageSeverities,
			.messageType = s_MessageTypes,
			.pfnUserCallback = DebugMessengerCallback,
		};

		VkInstanceCreateInfo instanceInfo =
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = s_CurrentConfig == Configuration::Release ? nullptr : &messengerCreateInfo,
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = uint32_t(enabledLayers.size()),
			.ppEnabledLayerNames = enabledLayers.data(),
			.enabledExtensionCount = uint32_t(enabledInstanceExtensions.size()),
			.ppEnabledExtensionNames = enabledInstanceExtensions.data(),
		};

		YUKI_VERIFY(vkCreateInstance(&instanceInfo, nullptr, &m_Instance) == VK_SUCCESS);
		volkLoadInstanceOnly(m_Instance);

		if constexpr (s_CurrentConfig != Configuration::Release)
			SetupDebugUtilsMessenger();

		SelectSuitablePhysicalDevice();
		CreateLogicalDevice(enabledLayers);

		{
			VmaVulkanFunctions vmaVulkanFunctions =
			{
				.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
				.vkGetDeviceProcAddr = vkGetDeviceProcAddr,
			};

			VmaAllocatorCreateInfo allocatorCreateInfo =
			{
				.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
				.physicalDevice = m_PhysicalDevice,
				.device = m_LogicalDevice,
				.pVulkanFunctions = &vmaVulkanFunctions,
				.instance = m_Instance,
				.vulkanApiVersion = VK_API_VERSION_1_3
			};

			vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator);
		}

		m_TransferScheduler = Unique<TransferScheduler>::Create(this);

		glslang::InitializeProcess();
	}

	VulkanRenderContext::~VulkanRenderContext()
	{
		DeviceWaitIdle();

		glslang::FinalizeProcess();

#define YUKI_DESTROY_REGISTRY(registry) registry.ForEach([&](auto key, auto& value) { Destroy(key); })
		YUKI_DESTROY_REGISTRY(m_DescriptorSetLayouts);
		YUKI_DESTROY_REGISTRY(m_Pipelines);
		YUKI_DESTROY_REGISTRY(m_Shaders);
		YUKI_DESTROY_REGISTRY(m_Samplers);
		YUKI_DESTROY_REGISTRY(m_ImageViews);
		YUKI_DESTROY_REGISTRY(m_Images);
		YUKI_DESTROY_REGISTRY(m_CommandPools);
		YUKI_DESTROY_REGISTRY(m_Buffers);
		YUKI_DESTROY_REGISTRY(m_Swapchains);
#undef YUKI_DESTROY_REGISTRY
	}

	uint32_t VulkanRenderContext::SelectQueue(VkQueueFlags InQueueFlags) const
	{
		DynamicArray<VkQueueFamilyProperties> queueFamilies;
		VulkanHelper::Enumerate(vkGetPhysicalDeviceQueueFamilyProperties, queueFamilies, m_PhysicalDevice);

		uint32_t bestQueueScore = std::numeric_limits<uint32_t>::max(); // Lower is better
		uint32_t bestQueue = uint32_t(queueFamilies.size());

		for (uint32_t familyIndex = 0; familyIndex < queueFamilies.size(); familyIndex++)
		{
			const auto& queueFamily = queueFamilies[familyIndex];

			if ((queueFamily.queueFlags & InQueueFlags) != InQueueFlags)
				continue;

			uint32_t score = std::popcount(queueFamily.queueFlags & ~uint32_t(InQueueFlags));

			if (score < bestQueueScore)
			{
				bestQueueScore = score;
				bestQueue = familyIndex;
			}
		}

		YUKI_VERIFY(bestQueue != queueFamilies.size());
		return bestQueue;
	}

	void VulkanRenderContext::CreateLogicalDevice(const DynamicArray<const char*>& InDeviceLayers)
	{
		uint32_t graphicsQueue = SelectQueue(VK_QUEUE_GRAPHICS_BIT);
		uint32_t transferQueue = SelectQueue(VK_QUEUE_TRANSFER_BIT);

		DynamicArray<VkDeviceQueueCreateInfo> queueCreateInfos;
		queueCreateInfos.resize(2);
		auto graphicsQueuePriorities = std::array{ 1.0f, 1.0f, 1.0f, 1.0f };
		queueCreateInfos[0] =
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = graphicsQueue,
			.queueCount = 4,
			.pQueuePriorities = graphicsQueuePriorities.data(),
		};

		auto transferQueuePriority = std::array{ 1.0f, 1.0f };
		queueCreateInfos[1] =
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = transferQueue,
			.queueCount = 2,
			.pQueuePriorities = transferQueuePriority.data(),
		};

		DynamicArray<const char*> deviceExtensions;
		deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		VkPhysicalDeviceVulkan13Features features13 =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.synchronization2 = VK_TRUE,
			.dynamicRendering = VK_TRUE,
		};

		VkPhysicalDeviceVulkan12Features features12 =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &features13,
			.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
			.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
			.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
			.descriptorBindingPartiallyBound = VK_TRUE,
			.runtimeDescriptorArray = VK_TRUE,
			.scalarBlockLayout = VK_TRUE,
			.imagelessFramebuffer = VK_TRUE,
			.timelineSemaphore = VK_TRUE,
			.bufferDeviceAddress = VK_TRUE,
		};

		VkPhysicalDeviceFeatures2 features2 =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &features12,
			.features = {
				.fillModeNonSolid = VK_TRUE,
				.shaderInt64 = VK_TRUE,
			},
		};

		VkDeviceCreateInfo deviceInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features2,
			.queueCreateInfoCount = uint32_t(queueCreateInfos.size()),
			.pQueueCreateInfos = queueCreateInfos.data(),
			.enabledLayerCount = uint32_t(InDeviceLayers.size()),
			.ppEnabledLayerNames = InDeviceLayers.data(),
			.enabledExtensionCount = uint32_t(deviceExtensions.size()),
			.ppEnabledExtensionNames = deviceExtensions.data(),
			.pEnabledFeatures = nullptr,
		};

		YUKI_VERIFY(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_LogicalDevice) == VK_SUCCESS);
		volkLoadDevice(m_LogicalDevice);

		for (uint32_t i = 0; i < queueCreateInfos[0].queueCount; i++)
		{
			auto[queueHandle, queue] = m_Queues.Acquire();
			queue.FamilyIndex = graphicsQueue;
			vkGetDeviceQueue(m_LogicalDevice, graphicsQueue, i, &queue.Queue);
			m_GraphicsQueues.emplace_back(queueHandle);
		}

		{
			auto[queueHandle, queue] = m_Queues.Acquire();
			queue.FamilyIndex = transferQueue;
			vkGetDeviceQueue(m_LogicalDevice, transferQueue, 0, &queue.Queue);
			m_TransferQueues.emplace_back(queueHandle);
		}

		{
			auto[queueHandle, queue] = m_Queues.Acquire();
			queue.FamilyIndex = transferQueue;
			vkGetDeviceQueue(m_LogicalDevice, transferQueue, 1, &queue.Queue);
			m_TransferQueues.emplace_back(queueHandle);
		}

		m_QueueFamilies.emplace_back(graphicsQueue);
		m_QueueFamilies.emplace_back(transferQueue);
	}

	void VulkanRenderContext::DeviceWaitIdle() const
	{
		vkDeviceWaitIdle(m_LogicalDevice);
	}

	DynamicArray<SwapchainHandle> VulkanRenderContext::GetSwapchains() const
	{
		DynamicArray<SwapchainHandle> result;
		result.reserve(m_Swapchains.GetCount());
		m_Swapchains.ForEach([&](auto key, const auto& element)
		{
			if (!m_Swapchains.IsValid(key))
				return;
			
			result.emplace_back(key);
		});
		return result;
	}

	static uint32_t CalculatePhysicalDeviceScore(VkPhysicalDevice InPhysicalDevice)
	{
		uint32_t score = 0;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(InPhysicalDevice, &deviceProperties);

		switch (deviceProperties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		{
			score += 5;
			break;
		}
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		{
			score += 10;
			break;
		}
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		{
			score += 3;
			break;
		}
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
		{
			score += 1;
			break;
		}
		default:
		{
			YUKI_VERIFY(false, "Unsupported Physical Device!");
			break;
		}
		}

		VkPhysicalDeviceVulkan13Features features13 =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
		};

		VkPhysicalDeviceFeatures2 features2 =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &features13
		};
		vkGetPhysicalDeviceFeatures2(InPhysicalDevice, &features2);

		if (features13.dynamicRendering == VK_TRUE)
			score += 10;

		if (features13.synchronization2 == VK_TRUE)
			score += 10;

		return score;
	}

	void VulkanRenderContext::SelectSuitablePhysicalDevice()
	{
		DynamicArray<VkPhysicalDevice> availableDevices;
		VulkanHelper::Enumerate(vkEnumeratePhysicalDevices, availableDevices, m_Instance);

		size_t selectedDeviceIndex = 0;
		uint32_t highestDeviceScore = 0;

		for (size_t i = 0; i < availableDevices.size(); i++)
		{
			uint32_t deviceScore = CalculatePhysicalDeviceScore(availableDevices[i]);

			if (deviceScore > highestDeviceScore)
			{
				highestDeviceScore = deviceScore;
				selectedDeviceIndex = i;
			}
		}

		m_PhysicalDevice = availableDevices[selectedDeviceIndex];
	}

	void VulkanRenderContext::SetupDebugUtilsMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity = s_MessageSeverities,
			.messageType = s_MessageTypes,
			.pfnUserCallback = DebugMessengerCallback,
		};

		vkCreateDebugUtilsMessengerEXT(m_Instance, &messengerCreateInfo, nullptr, &m_DebugUtilsMessengerHandle);
	}

}
