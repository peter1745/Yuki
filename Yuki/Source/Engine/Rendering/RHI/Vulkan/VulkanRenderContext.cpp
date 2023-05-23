#include "VulkanRenderContext.hpp"
#include "VulkanHelper.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanShaderCompiler.hpp"
#include "VulkanImage2D.hpp"

#include "Rendering/RenderTarget.hpp"

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

		volkLoadInstanceOnly(m_Instance);

		SelectSuitablePhysicalDevice();
		CreateLogicalDevice(enabledLayers);

		m_Allocator.Initialize(m_Instance, m_PhysicalDevice, m_Device);

		m_ShaderManager = Unique<ShaderManager>::Create();
		m_ShaderCompiler = Unique<VulkanShaderCompiler>::Create(m_ShaderManager.GetPtr(), m_Device);
	}

	void VulkanRenderContext::Destroy()
	{
		vkDeviceWaitIdle(m_Device);

		m_Allocator.Destroy();

		vkDestroyDevice(m_Device, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
	}

	Swapchain* VulkanRenderContext::CreateSwapchain(GenericWindow* InWindow)
	{
		return VulkanSwapchain::Create(this, InWindow);
	}

	Image2D* VulkanRenderContext::CreateImage2D(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat)
	{
		return VulkanImage2D::Create(this, InWidth, InHeight, InFormat);
	}

	ImageView2D* VulkanRenderContext::CreateImageView2D(Image2D* InImage)
	{
		return VulkanImageView2D::Create(this, (VulkanImage2D*)InImage);
	}

	RenderTarget* VulkanRenderContext::CreateRenderTarget(const RenderTargetInfo& InInfo)
	{
		return RenderTarget::Create(this, InInfo);
	}

	void VulkanRenderContext::DestroySwapchain(Swapchain* InSwapchain)
	{
		VulkanSwapchain::Destroy(this, (VulkanSwapchain*)InSwapchain);
	}

	void VulkanRenderContext::DestroyImage2D(Image2D* InImage)
	{
		VulkanImage2D::Destroy(this, (VulkanImage2D*)InImage);
	}

	void VulkanRenderContext::DestroyImageView2D(ImageView2D* InImageView)
	{
		VulkanImageView2D::Destroy(this, (VulkanImageView2D*)InImageView);
	}

	void VulkanRenderContext::DestroyRenderTarget(RenderTarget* InRenderTarget)
	{
		RenderTarget::Destroy(this, InRenderTarget);
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

		VkPhysicalDeviceVulkan13Features features13 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };

		VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extendedDynamicStateFeatures = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
			.pNext = &features13
		};

		VkPhysicalDeviceFeatures2 features2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &extendedDynamicStateFeatures };
		vkGetPhysicalDeviceFeatures2(InPhysicalDevice, &features2);

		if (features13.dynamicRendering == VK_TRUE)
			score += 10;

		if (features13.synchronization2 == VK_TRUE)
			score += 10;

		if (extendedDynamicStateFeatures.extendedDynamicState3PolygonMode == VK_TRUE)
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
		VkDeviceQueueCreateInfo queueCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = selectedQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority,
		};

		List<const char*> deviceExtensions;
		deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		deviceExtensions.emplace_back(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);

		VkPhysicalDeviceVulkan13Features features13 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features13.dynamicRendering = VK_TRUE;
		features13.synchronization2 = VK_TRUE;

		VkPhysicalDeviceExtendedDynamicState3FeaturesEXT extendedDynamicState3Features = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT,
			.pNext = &features13,
			.extendedDynamicState3PolygonMode = VK_TRUE,
		};

		VkPhysicalDeviceFeatures2 features2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &extendedDynamicState3Features };

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

		YUKI_VERIFY(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device) == VK_SUCCESS);
		volkLoadDevice(m_Device);

		m_GraphicsQueue = Unique<VulkanQueue>::Create();
		vkGetDeviceQueue(m_Device, selectedQueueFamilyIndex, 0, &m_GraphicsQueue->m_Queue);
		m_GraphicsQueue->m_QueueFamily = selectedQueueFamilyIndex;
	}

}
