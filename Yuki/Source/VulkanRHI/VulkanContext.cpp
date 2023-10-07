#include "VulkanRHI.hpp"
#include "VulkanUtils.hpp"
#include "VulkanRenderFeatures.hpp"
#include "VulkanPlatformUtils.hpp"

namespace Yuki::RHI {

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

	DynamicArray<const char*> GetDeviceSupportedExtensions(VkPhysicalDevice InPhysicalDevice, const HashMap<RendererFeature, Unique<VulkanFeature>>& InRequestedFeatures)
	{
		DynamicArray<const char*> Result;

		DynamicArray<VkExtensionProperties> DeviceExtensions;
		Vulkan::Enumerate(vkEnumerateDeviceExtensionProperties, DeviceExtensions, InPhysicalDevice, nullptr);

		for (const auto& Feature : InRequestedFeatures | std::views::values)
		{
			for (const auto& Extension : Feature->GetRequiredExtensions())
			{
				std::ranges::for_each(DeviceExtensions, [&Extension, &Result](const auto& InDeviceExtension)
					{
						if (Extension == InDeviceExtension.extensionName)
							Result.push_back(Extension.data());
					});
			}
		}

		return Result;
	}

	uint32_t CalculateScoreForDeviceType(const VkPhysicalDeviceProperties& InProperties)
	{
		switch (InProperties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return 5;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return 10;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return 3;
		case VK_PHYSICAL_DEVICE_TYPE_CPU: return 1;
		default: return 0;
		}
	}

	void RemoveUnsupportedFeatures(VkPhysicalDevice InPhysicalDevice, HashMap<RendererFeature, Unique<VulkanFeature>>& InFeatures)
	{
		DynamicArray<VkExtensionProperties> DeviceExtensions;
		Vulkan::Enumerate(vkEnumerateDeviceExtensionProperties, DeviceExtensions, InPhysicalDevice, nullptr);

		for (auto It = InFeatures.begin(); It != InFeatures.end(); It++)
		{
			for (const auto& Extension : It->second->GetRequiredExtensions())
			{
				bool IsSupported = false;

				for (const auto& DeviceExtension : DeviceExtensions)
				{
					if (Extension == DeviceExtension.extensionName)
					{
						IsSupported = true;
						break;
					}
				}

				if (!IsSupported)
				{
					It = InFeatures.erase(It);
					break;
				}
			}
		}
	}

	Context Context::Create(Config InConfig)
	{
		auto context = new Impl();

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

			context->ValidationSupport = ValidationSupported;
		}

		VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
		};

		{
			DynamicArray<const char*> EnabledInstanceLayers;
			DynamicArray<const char*> EnabledInstanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, };

			Vulkan::AddPlatformInstanceExtensions(EnabledInstanceExtensions);

			if (context->ValidationSupport)
			{
				EnabledInstanceLayers.emplace_back("VK_LAYER_KHRONOS_validation");
				EnabledInstanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

				constexpr uint32_t MessageSeverities =
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

				constexpr uint32_t MessageTypes =
					VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

				MessengerCreateInfo.messageSeverity = MessageSeverities;
				MessengerCreateInfo.messageType = MessageTypes;
				MessengerCreateInfo.pfnUserCallback = DebugMessengerCallback;
			}

			VkApplicationInfo AppInfo =
			{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.apiVersion = VK_API_VERSION_1_3
			};

			VkInstanceCreateInfo InstanceInfo =
			{
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pNext = &MessengerCreateInfo,
				.flags = 0,
				.pApplicationInfo = &AppInfo,
				.enabledLayerCount = static_cast<uint32_t>(EnabledInstanceLayers.size()),
				.ppEnabledLayerNames = EnabledInstanceLayers.data(),
				.enabledExtensionCount = static_cast<uint32_t>(EnabledInstanceExtensions.size()),
				.ppEnabledExtensionNames = EnabledInstanceExtensions.data(),
			};

			vkCreateInstance(&InstanceInfo, nullptr, &context->Instance);

			volkLoadInstanceOnly(context->Instance);
		}

		if (context->ValidationSupport)
			vkCreateDebugUtilsMessengerEXT(context->Instance, &MessengerCreateInfo, nullptr, &context->DebugMessengerHandle);

		{
			context->EnabledFeatures[RendererFeature::Core] = std::move(Unique<VulkanCoreFeature>::New());

			for (auto RequestedFeature : InConfig.RequestedFeatures)
				context->EnabledFeatures[RequestedFeature] = std::move(Vulkan::GetVulkanFeature(RequestedFeature));

			{
				DynamicArray<VkPhysicalDevice> Devices;
				Vulkan::Enumerate(vkEnumeratePhysicalDevices, Devices, context->Instance);

				auto SupportedExtensions = DynamicArray<const char*>{};

				uint32_t MaxScore = 0;
				size_t BestDeviceIndex = std::numeric_limits<size_t>::max();

				for (size_t Index = 0; Index < Devices.size(); Index++)
				{
					auto Device = Devices[Index];

					VkPhysicalDeviceProperties Properties;
					vkGetPhysicalDeviceProperties(Device, &Properties);

					SupportedExtensions = GetDeviceSupportedExtensions(Device, context->EnabledFeatures);

					uint32_t Score = CalculateScoreForDeviceType(Properties);
					Score += Cast<uint32_t>(SupportedExtensions.size());

					if (Score > MaxScore)
					{
						MaxScore = Score;
						BestDeviceIndex = Index;
					}
				}

				YUKI_VERIFY(BestDeviceIndex < Devices.size());

				context->PhysicalDevice = Devices[BestDeviceIndex];

				RemoveUnsupportedFeatures(context->PhysicalDevice, context->EnabledFeatures);
			}

			{
				DynamicArray<VkDeviceQueueCreateInfo> QueueCreateInfos;
				DynamicArray<float> QueuePriorities;

				DynamicArray<VkQueueFamilyProperties> QueueFamilies;
				Vulkan::Enumerate(vkGetPhysicalDeviceQueueFamilyProperties, QueueFamilies, context->PhysicalDevice);
				uint32_t QueueFamilyIndex = 0;
				for (const auto& QueueFamily : QueueFamilies)
				{
					for (uint32_t Index = 0; Index < QueueFamily.queueCount; Index++)
					{
						auto Queue = new QueueRH::Impl();
						Queue->Family = QueueFamilyIndex;
						Queue->Index = Index;
						Queue->Flags = QueueFamily.queueFlags;
						QueuePriorities.push_back(1.0f);

						context->Queues.push_back(Queue);
					}

					QueueFamilyIndex++;
				}

				size_t QueuePrioritiesStart = 0;
				for (uint32_t Index = 0; Index < QueueFamilyIndex; Index++)
				{
					QueueCreateInfos.push_back({
						.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
						.pNext = nullptr,
						.flags = 0,
						.queueFamilyIndex = Index,
						.queueCount = QueueFamilies[Index].queueCount,
						.pQueuePriorities = &QueuePriorities[QueuePrioritiesStart],
					});

					QueuePrioritiesStart += QueueFamilies[Index].queueCount;
				}

				VkPhysicalDeviceFeatures2 Features2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, };
				for (const auto& RequestedFeature : context->EnabledFeatures | std::views::values)
					RequestedFeature->PopulatePhysicalDeviceFeatures(Features2);

				DynamicArray<const char*> RequiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
				for (const auto& Feature : context->EnabledFeatures | std::views::values)
				{
					for (auto Extension : Feature->GetRequiredExtensions())
						RequiredExtensions.push_back(Extension.data());
				}

				VkDeviceCreateInfo DeviceInfo =
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
					.pNext = &Features2,
					.flags = 0,
					.queueCreateInfoCount = Cast<uint32_t>(QueueCreateInfos.size()),
					.pQueueCreateInfos = QueueCreateInfos.data(),
					.enabledLayerCount = 0,
					.ppEnabledLayerNames = nullptr,
					.enabledExtensionCount = Cast<uint32_t>(RequiredExtensions.size()),
					.ppEnabledExtensionNames = RequiredExtensions.data(),
					.pEnabledFeatures = nullptr,
				};
				YUKI_VERIFY(vkCreateDevice(context->PhysicalDevice, &DeviceInfo, nullptr, &context->Device) == VK_SUCCESS);

				volkLoadDevice(context->Device);

				VkPhysicalDeviceProperties2 Properties2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, };
				for (const auto& RequestedFeature : context->EnabledFeatures | std::views::values)
					RequestedFeature->PopulateProperties(Properties2);
				vkGetPhysicalDeviceProperties2(context->PhysicalDevice, &Properties2);

				for (auto Queue : context->Queues)
					vkGetDeviceQueue(context->Device, Queue->Family, Queue->Index, &Queue->Handle);
			}

			VmaVulkanFunctions VulkanFunctions = {};
			VulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
			VulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;

			VmaAllocatorCreateInfo AllocatorInfo =
			{
				.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
				.physicalDevice = context->PhysicalDevice,
				.device = context->Device,
				.pVulkanFunctions = &VulkanFunctions,
				.instance = context->Instance,
				.vulkanApiVersion = VK_API_VERSION_1_3,
			};
			YUKI_VERIFY(vmaCreateAllocator(&AllocatorInfo, &context->Allocator) == VK_SUCCESS);
		}

		return { context };
	}

	/*VulkanContext::~VulkanContext() noexcept
	{
		if (m_DebugMessengerHandle)
			vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessengerHandle, nullptr);

		vkDestroyInstance(m_Instance, nullptr);
	}*/

}
