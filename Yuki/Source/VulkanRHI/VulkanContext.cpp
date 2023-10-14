#include "VulkanRHI.hpp"
#include "VulkanUtils.hpp"
#include "VulkanRenderFeatures.hpp"
#include "VulkanPlatformUtils.hpp"
#include "VulkanShaderCompiler.hpp"

namespace Yuki::RHI {

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
	{
		if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			Logging::Info("{}", callbackData->pMessage);
		}
		else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			Logging::Warn("{}", callbackData->pMessage);
		}
		else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			Logging::Error("{}", callbackData->pMessage);
			YUKI_VERIFY(false);
		}

		return VK_FALSE;
	}

	DynamicArray<const char*> GetDeviceSupportedExtensions(VkPhysicalDevice physicalDevice, const HashMap<RendererFeature, Unique<VulkanFeature>>& requestedFeatures)
	{
		DynamicArray<const char*> result;

		DynamicArray<VkExtensionProperties> deviceExtensions;
		Vulkan::Enumerate(vkEnumerateDeviceExtensionProperties, deviceExtensions, physicalDevice, nullptr);

		for (const auto& feature : requestedFeatures | std::views::values)
		{
			for (const auto& extension : feature->GetRequiredExtensions())
			{
				std::ranges::for_each(deviceExtensions, [&extension, &result](const auto& deviceExtension)
				{
					if (extension == deviceExtension.extensionName)
						result.push_back(extension.data());
				});
			}
		}

		return result;
	}

	uint32_t CalculateScoreForDeviceType(const VkPhysicalDeviceProperties& properties)
	{
		switch (properties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return 5;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return 10;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return 3;
		case VK_PHYSICAL_DEVICE_TYPE_CPU: return 1;
		default: return 0;
		}
	}

	void RemoveUnsupportedFeatures(VkPhysicalDevice physicalDevice, HashMap<RendererFeature, Unique<VulkanFeature>>& features)
	{
		DynamicArray<VkExtensionProperties> deviceExtensions;
		Vulkan::Enumerate(vkEnumerateDeviceExtensionProperties, deviceExtensions, physicalDevice, nullptr);

		for (auto it = features.begin(); it != features.end(); it++)
		{
			for (const auto& extension : it->second->GetRequiredExtensions())
			{
				bool supported = false;

				for (const auto& deviceExtension : deviceExtensions)
				{
					if (extension == deviceExtension.extensionName)
					{
						supported = true;
						break;
					}
				}

				if (!supported)
				{
					it = features.erase(it);
					break;
				}
			}
		}
	}

	Context Context::Create(Config config)
	{
		auto context = new Impl();

		volkInitialize();

		// Check if validation is available, disable if it's not
		{
			bool validationSupported = false;

			DynamicArray<VkLayerProperties> layers;
			Vulkan::Enumerate(vkEnumerateInstanceLayerProperties, layers);
			for (const auto& layer : layers)
			{
				if (std::string_view(layer.layerName) == "VK_LAYER_KHRONOS_validation")
				{
					validationSupported = true;
					break;
				}
			}

			context->ValidationSupport = validationSupported;
		}

		VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
		};

		{
			DynamicArray<const char*> enabledInstanceLayers;
			DynamicArray<const char*> enabledInstanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, };

			Vulkan::AddPlatformInstanceExtensions(enabledInstanceExtensions);

			if (context->ValidationSupport)
			{
				enabledInstanceLayers.emplace_back("VK_LAYER_KHRONOS_validation");
				enabledInstanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

				constexpr uint32_t MessageSeverities =
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

				constexpr uint32_t MessageTypes =
					VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
					VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

				messengerCreateInfo.messageSeverity = MessageSeverities;
				messengerCreateInfo.messageType = MessageTypes;
				messengerCreateInfo.pfnUserCallback = DebugMessengerCallback;
			}

			VkApplicationInfo appInfo =
			{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.apiVersion = VK_API_VERSION_1_3
			};

			VkInstanceCreateInfo instanceInfo =
			{
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pNext = &messengerCreateInfo,
				.flags = 0,
				.pApplicationInfo = &appInfo,
				.enabledLayerCount = static_cast<uint32_t>(enabledInstanceLayers.size()),
				.ppEnabledLayerNames = enabledInstanceLayers.data(),
				.enabledExtensionCount = static_cast<uint32_t>(enabledInstanceExtensions.size()),
				.ppEnabledExtensionNames = enabledInstanceExtensions.data(),
			};

			vkCreateInstance(&instanceInfo, nullptr, &context->Instance);

			volkLoadInstanceOnly(context->Instance);
		}

		if (context->ValidationSupport)
			vkCreateDebugUtilsMessengerEXT(context->Instance, &messengerCreateInfo, nullptr, &context->DebugMessengerHandle);

		{
			context->EnabledFeatures[RendererFeature::Core] = std::move(Unique<VulkanCoreFeature>::New());

			for (auto requestedFeature : config.RequestedFeatures)
				context->EnabledFeatures[requestedFeature] = std::move(Vulkan::GetVulkanFeature(requestedFeature));

			{
				DynamicArray<VkPhysicalDevice> devices;
				Vulkan::Enumerate(vkEnumeratePhysicalDevices, devices, context->Instance);

				auto supportedExtensions = DynamicArray<const char*>{};

				uint32_t maxScore = 0;
				size_t bestDeviceIndex = std::numeric_limits<size_t>::max();

				for (size_t i = 0; i < devices.size(); i++)
				{
					auto device = devices[i];

					VkPhysicalDeviceProperties properties;
					vkGetPhysicalDeviceProperties(device, &properties);

					supportedExtensions = GetDeviceSupportedExtensions(device, context->EnabledFeatures);

					uint32_t Score = CalculateScoreForDeviceType(properties);
					Score += Cast<uint32_t>(supportedExtensions.size());

					if (Score > maxScore)
					{
						maxScore = Score;
						bestDeviceIndex = i;
					}
				}

				YUKI_VERIFY(bestDeviceIndex < devices.size());

				context->PhysicalDevice = devices[bestDeviceIndex];

				RemoveUnsupportedFeatures(context->PhysicalDevice, context->EnabledFeatures);
			}

			{
				DynamicArray<VkDeviceQueueCreateInfo> queueCreateInfos;
				DynamicArray<float> queuePriorities;

				DynamicArray<VkQueueFamilyProperties> queueFamilies;
				Vulkan::Enumerate(vkGetPhysicalDeviceQueueFamilyProperties, queueFamilies, context->PhysicalDevice);
				uint32_t queueFamilyIndex = 0;
				for (const auto& queueFamily : queueFamilies)
				{
					for (uint32_t i = 0; i < queueFamily.queueCount; i++)
					{
						auto queue = new QueueRH::Impl();
						queue->Ctx = { context };
						queue->Family = queueFamilyIndex;
						queue->Index = i;
						queue->Flags = queueFamily.queueFlags;
						queuePriorities.push_back(1.0f);

						context->Queues.push_back(queue);
					}

					queueFamilyIndex++;
				}

				size_t queuePrioritiesStart = 0;
				for (uint32_t i = 0; i < queueFamilyIndex; i++)
				{
					queueCreateInfos.push_back({
						.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
						.pNext = nullptr,
						.flags = 0,
						.queueFamilyIndex = i,
						.queueCount = queueFamilies[i].queueCount,
						.pQueuePriorities = &queuePriorities[queuePrioritiesStart],
					});

					queuePrioritiesStart += queueFamilies[i].queueCount;
				}

				VkPhysicalDeviceFeatures2 features2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, };
				features2.features.shaderInt64 = VK_TRUE;
				for (const auto& requestedFeature : context->EnabledFeatures | std::views::values)
					requestedFeature->PopulatePhysicalDeviceFeatures(features2);

				DynamicArray<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
				for (const auto& feature : context->EnabledFeatures | std::views::values)
				{
					for (auto extension : feature->GetRequiredExtensions())
						requiredExtensions.push_back(extension.data());
				}

				VkDeviceCreateInfo deviceInfo =
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
					.pNext = &features2,
					.flags = 0,
					.queueCreateInfoCount = Cast<uint32_t>(queueCreateInfos.size()),
					.pQueueCreateInfos = queueCreateInfos.data(),
					.enabledLayerCount = 0,
					.ppEnabledLayerNames = nullptr,
					.enabledExtensionCount = Cast<uint32_t>(requiredExtensions.size()),
					.ppEnabledExtensionNames = requiredExtensions.data(),
					.pEnabledFeatures = nullptr,
				};
				YUKI_VK_CHECK(vkCreateDevice(context->PhysicalDevice, &deviceInfo, nullptr, &context->Device));

				volkLoadDevice(context->Device);

				VkPhysicalDeviceProperties2 properties2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, };
				for (const auto& requestedFeature : context->EnabledFeatures | std::views::values)
					requestedFeature->PopulateProperties(properties2);
				vkGetPhysicalDeviceProperties2(context->PhysicalDevice, &properties2);

				for (auto queue : context->Queues)
					vkGetDeviceQueue(context->Device, queue->Family, queue->Index, &queue->Handle);
			}

			VmaVulkanFunctions vulkanFunctions = {};
			vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
			vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;

			VmaAllocatorCreateInfo allocatorInfo =
			{
				.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
				.physicalDevice = context->PhysicalDevice,
				.device = context->Device,
				.pVulkanFunctions = &vulkanFunctions,
				.instance = context->Instance,
				.vulkanApiVersion = VK_API_VERSION_1_3,
			};
			YUKI_VK_CHECK(vmaCreateAllocator(&allocatorInfo, &context->Allocator));
		}

		context->ShaderCompiler = Unique<VulkanShaderCompiler>::New();

		{
			VkMutableDescriptorTypeListEXT mutableDescriptorList =
			{
				.descriptorTypeCount = 3,
				.pDescriptorTypes = std::array {
					VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
					VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
					VK_DESCRIPTOR_TYPE_SAMPLER
				}.data()
			};

			VkMutableDescriptorTypeCreateInfoEXT mutableDescriptorInfo =
			{
				.sType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT,
				.pNext = nullptr,
				.mutableDescriptorTypeListCount = 1,
				.pMutableDescriptorTypeLists = &mutableDescriptorList,
			};

			VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo =
			{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
				.pNext = &mutableDescriptorInfo,
				.bindingCount = 1,
				.pBindingFlags = std::array {
					VkDescriptorSetLayoutCreateFlags(
						VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
						VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
						VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
					)
				}.data(),
			};

			VkDescriptorSetLayoutCreateInfo layoutInfo =
			{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext = &bindingFlagsInfo,
				.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
				.bindingCount = 1,
				.pBindings = std::array {
					VkDescriptorSetLayoutBinding{
						.binding = 0,
						.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_EXT,
						.descriptorCount = 65536, // TODO(Peter): Use the smallest of the possible descriptor limits
						.stageFlags = VK_SHADER_STAGE_ALL,
						.pImmutableSamplers = nullptr,
					}
				}.data(),
			};

			vkCreateDescriptorSetLayout(context->Device, &layoutInfo, nullptr, &context->DescriptorHeapLayout);
		}

		Context result = { context };
		context->TemporariesPool = CommandPool::Create(result, result.RequestQueue(QueueType::Graphics));
		return result;
	}

	CommandList Context::Impl::GetTemporaryCommandList()
	{
		if (TemporariesPool->AllocatedLists.size() >= 6)
			TemporariesPool.Reset();

		auto cmd = TemporariesPool.NewList();
		cmd.Begin();
		return cmd;
	}

	void Context::Impl::EndTemporaryCommandList(CommandList cmd)
	{
		cmd.End();

		auto ctx = Context{ this };
		auto queue = ctx.RequestQueue(QueueType::Graphics);
		auto fence = Fence::Create(ctx);

		queue.Submit({ cmd }, {}, { fence });
		fence.Wait();
		fence.Destroy();
	}

	/*VulkanContext::~VulkanContext() noexcept
	{
		if (m_DebugMessengerHandle)
			vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessengerHandle, nullptr);

		vkDestroyInstance(m_Instance, nullptr);
	}*/

}
