#include "VulkanRenderContext.hpp"
#include "VulkanHelper.hpp"
#include "VulkanViewport.hpp"
#include "VulkanShaderCompiler.hpp"
#include "VulkanImage2D.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanFence.hpp"
#include "VulkanGraphicsPipelineBuilder.hpp"
#include "VulkanRenderInterface.hpp"
#include "VulkanCommandBufferPool.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanSampler.hpp"
#include "VulkanSetLayoutBuilder.hpp"

#define VK_VERIFY(res) if (res != VK_SUCCESS) { LogError("Vulkan Validation failed: {}", int32_t(res)); }

namespace Yuki {

	static constexpr uint32_t MessageSeverities = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	static constexpr uint32_t MessageTypes = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

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
			std::cin.get();
			YUKI_VERIFY(false);
		}

		return VK_FALSE;
	}

	VulkanRenderContext::VulkanRenderContext()
	{
		YUKI_VERIFY(volkInitialize() == VK_SUCCESS);
	}

	void VulkanRenderContext::Initialize()
	{
		List<const char*> enabledLayers;
		List<const char*> enabledInstanceExtensions;

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
			.messageSeverity = MessageSeverities,
			.messageType = MessageTypes,
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

		m_Allocator.Initialize(m_Instance, m_PhysicalDevice, m_Device);

		VkCommandPoolCreateInfo poolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = m_GraphicsQueue->GetFamilyIndex()
		};
		vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool);

		// Create Transient Command Buffer Pool
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_TransientCommandPool);

		m_ShaderManager = Unique<ShaderManager>::Create();
		m_ShaderCompiler = Unique<VulkanShaderCompiler>::Create(m_ShaderManager.GetPtr(), m_Device);
	}

	void VulkanRenderContext::Destroy()
	{
		vkDeviceWaitIdle(m_Device);
		
		vkDestroyCommandPool(m_Device, m_TransientCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

		m_Allocator.Destroy();

		vkDestroyDevice(m_Device, nullptr);
		vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugUtilsMessengerHandle, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
	}

	void VulkanRenderContext::ResetCommandPool()
	{
		vkResetCommandPool(m_Device, m_CommandPool, 0);
	}

	void VulkanRenderContext::WaitDeviceIdle() const
	{
		vkDeviceWaitIdle(m_Device);
	}

	RenderInterface* VulkanRenderContext::CreateRenderInterface() { return new VulkanRenderInterface(); }
	void VulkanRenderContext::DestroyRenderInterface(RenderInterface* InRenderInterface) { delete InRenderInterface; }

	GraphicsPipelineBuilder* VulkanRenderContext::CreateGraphicsPipelineBuilder() { return new VulkanGraphicsPipelineBuilder(this); }
	void VulkanRenderContext::DestroyGraphicsPipelineBuilder(GraphicsPipelineBuilder* InPipelineBuilder) { delete InPipelineBuilder; }

	SetLayoutBuilder* VulkanRenderContext::CreateSetLayoutBuilder() { return new VulkanSetLayoutBuilder(this); }
	void VulkanRenderContext::DestroySetLayoutBuilder(SetLayoutBuilder* InSetLayoutBuilder) { delete InSetLayoutBuilder; }

	DescriptorPool* VulkanRenderContext::CreateDescriptorPool(std::span<DescriptorCount> InDescriptorCounts) { return new VulkanDescriptorPool(this, InDescriptorCounts); }
	void VulkanRenderContext::DestroyDescriptorPool(DescriptorPool* InDescriptorPool) { delete InDescriptorPool; }

	Viewport* VulkanRenderContext::CreateViewport(GenericWindow* InWindow) { return new VulkanViewport(this, InWindow); }
	void VulkanRenderContext::DestroyViewport(Viewport* InViewport) { delete InViewport; }

	Image2D* VulkanRenderContext::CreateImage2D(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) { return new VulkanImage2D(this, InWidth, InHeight, InFormat, InUsage); }
	void VulkanRenderContext::DestroyImage2D(Image2D* InImage) { delete InImage; }

	ImageView2D* VulkanRenderContext::CreateImageView2D(Image2D* InImage) { return new VulkanImageView2D(this, (VulkanImage2D*)InImage); }
	void VulkanRenderContext::DestroyImageView2D(ImageView2D* InImageView) { delete InImageView; }

	Sampler* VulkanRenderContext::CreateSampler() { return new VulkanSampler(this); }
	void VulkanRenderContext::DestroySampler(Sampler* InSampler) { delete InSampler; }

	Buffer* VulkanRenderContext::CreateBuffer(const BufferInfo& InInfo) { return new VulkanBuffer(this, InInfo); }
	void VulkanRenderContext::DestroyBuffer(Buffer* InBuffer) { delete InBuffer; }

	Fence* VulkanRenderContext::CreateFence() { return new VulkanFence(this); }
	void VulkanRenderContext::DestroyFence(Fence* InFence) { delete InFence; }

	CommandBufferPool* VulkanRenderContext::CreateCommandBufferPool(CommandBufferPoolInfo InInfo) { return new VulkanCommandBufferPool(this, std::move(InInfo)); }
	void VulkanRenderContext::DestroyCommandBufferPool(CommandBufferPool* InCommandBuffer) { delete InCommandBuffer; }

	VkCommandBuffer VulkanRenderContext::CreateTransientCommandBuffer() const
	{
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

		VkCommandBufferAllocateInfo allocateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = m_TransientCommandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		vkAllocateCommandBuffers(m_Device, &allocateInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanRenderContext::DestroyTransientCommandBuffer(VkCommandBuffer InCommandBuffer) const
	{
		vkFreeCommandBuffers(m_Device, m_TransientCommandPool, 1, &InCommandBuffer);
	}

	VkSurfaceCapabilitiesKHR VulkanRenderContext::QuerySurfaceCapabilities(VkSurfaceKHR InSurface) const
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, InSurface, &surfaceCapabilities);
		return surfaceCapabilities;
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
		List<VkPhysicalDevice> availableDevices;
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

	void VulkanRenderContext::CreateLogicalDevice(const List<const char*>& InDeviceLayers)
	{
		uint32_t selectedQueueFamilyIndex = VulkanHelper::SelectGraphicsQueue(m_PhysicalDevice);

		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = selectedQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		};

		List<const char*> deviceExtensions;
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
			.timelineSemaphore = VK_TRUE,
		};

		VkPhysicalDeviceFeatures2 features2 =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &features12,
			.features =
			{
				.fillModeNonSolid = VK_TRUE,
			},
		};

		VkDeviceCreateInfo deviceInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features2,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledLayerCount = uint32_t(InDeviceLayers.size()),
			.ppEnabledLayerNames = InDeviceLayers.data(),
			.enabledExtensionCount = uint32_t(deviceExtensions.size()),
			.ppEnabledExtensionNames = deviceExtensions.data(),
			.pEnabledFeatures = nullptr,
		};

		VK_VERIFY(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device));
		volkLoadDevice(m_Device);

		m_GraphicsQueue = Unique<VulkanQueue>::Create();
		m_GraphicsQueue->m_Context = this;
		vkGetDeviceQueue(m_Device, selectedQueueFamilyIndex, 0, &m_GraphicsQueue->m_Queue);
		m_GraphicsQueue->m_QueueFamily = selectedQueueFamilyIndex;
	}

	void VulkanRenderContext::SetupDebugUtilsMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity = MessageSeverities,
			.messageType = MessageTypes,
			.pfnUserCallback = DebugMessengerCallback,
		};

		vkCreateDebugUtilsMessengerEXT(m_Instance, &messengerCreateInfo, nullptr, &m_DebugUtilsMessengerHandle);
	}

}
