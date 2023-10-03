#include "VulkanRenderDevice.hpp"
#include "VulkanUtils.hpp"
#include "VulkanRenderFeatures.hpp"

namespace Yuki::RHI {

	VulkanRenderDevice::VulkanRenderDevice(VkInstance InInstance, const DynamicArray<RendererFeature>& InRequestedFeatures)
		: m_Instance(InInstance)
	{
		for (auto RequestedFeature : InRequestedFeatures)
			m_Features[RequestedFeature] = std::move(Vulkan::GetVulkanFeature(RequestedFeature));

		FindPhysicalDevice();
		CreateLogicalDevice();
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

	void VulkanRenderDevice::RemoveUnsupportedFeatures()
	{
		DynamicArray<VkExtensionProperties> DeviceExtensions;
		Vulkan::Enumerate(vkEnumerateDeviceExtensionProperties, DeviceExtensions, m_PhysicalDevice, nullptr);

		for (auto It = m_Features.begin(); It != m_Features.end(); It++)
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
					It = m_Features.erase(It);
					break;
				}
			}
		}
	}

	DynamicArray<const char*> GetDeviceSupportedExtensions(VkPhysicalDevice InPhysicalDevice, const HashMap<RHI::RendererFeature, Unique<VulkanFeature>>& InRequestedFeatures)
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

	void VulkanRenderDevice::FindPhysicalDevice()
	{
		DynamicArray<VkPhysicalDevice> Devices;
		Vulkan::Enumerate(vkEnumeratePhysicalDevices, Devices, m_Instance);

		auto SupportedExtensions = DynamicArray<const char*>{};

		uint32_t MaxScore = 0;
		size_t BestDeviceIndex = std::numeric_limits<size_t>::max();

		for (size_t Index = 0; Index < Devices.size(); Index++)
		{
			auto Device = Devices[Index];

			VkPhysicalDeviceProperties Properties;
			vkGetPhysicalDeviceProperties(Device, &Properties);

			SupportedExtensions = GetDeviceSupportedExtensions(Device, m_Features);

			uint32_t Score = CalculateScoreForDeviceType(Properties);
			Score += Cast<uint32_t>(SupportedExtensions.size());

			if (Score > MaxScore)
			{
				MaxScore = Score;
				BestDeviceIndex = Index;
			}
		}

		YUKI_VERIFY(BestDeviceIndex < Devices.size());

		m_PhysicalDevice = Devices[BestDeviceIndex];
		RemoveUnsupportedFeatures();
	}

	uint32_t SelectQueue(VkPhysicalDevice InPhysicalDevice, VkQueueFlags InQueueFlags)
	{
		DynamicArray<VkQueueFamilyProperties> QueueFamilies;
		Vulkan::Enumerate(vkGetPhysicalDeviceQueueFamilyProperties, QueueFamilies, InPhysicalDevice);

		uint32_t BestScore = std::numeric_limits<uint32_t>::max();
		size_t BestQueue = std::numeric_limits<size_t>::max();

		for (size_t Index = 0; Index < QueueFamilies.size(); Index++)
		{
			const auto& QueueProperties = QueueFamilies[Index];

			if ((QueueProperties.queueFlags & InQueueFlags) != InQueueFlags)
				continue;

			uint32_t Score = std::popcount(QueueProperties.queueFlags & ~Cast<uint32_t>(InQueueFlags));

			if (Score < BestScore)
			{
				BestScore = Score;
				BestQueue = Index;
			}
		}

		YUKI_VERIFY(BestScore != std::numeric_limits<uint32_t>::max() && BestQueue < QueueFamilies.size());
		return Cast<uint32_t>(BestQueue);
	}

	DynamicArray<VkDeviceQueueCreateInfo> CreateQueueInfos(VkPhysicalDevice InPhysicalDevice, const std::initializer_list<VkQueueFlags>& InQueueFlags, DynamicArray<float>& InQueuePriorities)
	{
		DynamicArray<VkDeviceQueueCreateInfo> Result;
		Result.reserve(InQueueFlags.size());
		InQueuePriorities.reserve(InQueueFlags.size());

		for (auto QueueFlags : InQueueFlags)
		{
			uint32_t QueueIndex = SelectQueue(InPhysicalDevice, QueueFlags);

			Result.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = QueueIndex,
				.queueCount = 1,
				.pQueuePriorities = &InQueuePriorities.emplace_back(1.0f),
			});
		}

		return Result;
	}

	void VulkanRenderDevice::CreateLogicalDevice()
	{
		DynamicArray<VkDeviceQueueCreateInfo> QueueCreateInfos;
		DynamicArray<float> QueuePriorities;

		DynamicArray<VkQueueFamilyProperties> QueueFamilies;
		Vulkan::Enumerate(vkGetPhysicalDeviceQueueFamilyProperties, QueueFamilies, m_PhysicalDevice);
		uint32_t QueueFamilyIndex = 0;
		for (const auto& QueueFamily : QueueFamilies)
		{
			for (uint32_t Index = 0; Index < QueueFamily.queueCount; Index++)
			{
				auto[Handle, Queue] = m_Queues.Acquire();
				Queue.Family = QueueFamilyIndex;
				Queue.Index = Index;
				Queue.Flags = QueueFamily.queueFlags;
				QueuePriorities.push_back(1.0f);
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

		VkPhysicalDeviceVulkan13Features Features13 =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.synchronization2 = VK_TRUE,
			.dynamicRendering = VK_TRUE,
		};

		VkPhysicalDeviceVulkan12Features Features12 =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &Features13,
			.timelineSemaphore = VK_TRUE
		};

		VkPhysicalDeviceFeatures2 Features2 =
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &Features12,
		};

		for (const auto& RequestedFeature : m_Features | std::views::values)
			RequestedFeature->PopulatePhysicalDeviceFeatures(Features2);
		vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &Features2);

		DynamicArray<const char*> RequiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		for (const auto& Feature : m_Features | std::views::values)
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
		YUKI_VERIFY(vkCreateDevice(m_PhysicalDevice, &DeviceInfo, nullptr, &m_Device) == VK_SUCCESS);

		volkLoadDevice(m_Device);

		m_Queues.ForEach([this](auto Handle, auto& Queue)
		{
			vkGetDeviceQueue(m_Device, Queue.Family, Queue.Index, &Queue.Handle);
		});
	}

	bool VulkanRenderDevice::IsFeatureEnabled(RendererFeature InFeature) const
	{
		return m_Features.contains(InFeature);
	}

}
