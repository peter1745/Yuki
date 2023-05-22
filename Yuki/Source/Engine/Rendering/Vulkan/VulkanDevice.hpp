#pragma once

#include "Rendering/Device.hpp"

#include "Vulkan.hpp"

namespace Yuki {

	class VulkanDevice : public Device
	{
	public:
		VulkanDevice(VkPhysicalDevice InPhysicalDevice);

		void WaitIdle() const override;

		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkDevice GetLogicalDevice() const { return m_Device; }

		uint32_t GetQueueFamilyIndex() const { return m_QueueFamilyIndex; }
		VkQueue GetQueue() const { return m_Queue; }

		uint32_t GetDeviceScore() const { return m_DeviceScore; }

		const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const { return m_PhysicalDeviceProperties; }

		VkSurfaceCapabilitiesKHR QuerySurfaceCapabilities(VkSurfaceKHR InSurface) const;

	public:
		void CreateLogicalDevice(const List<const char*>& InDeviceLayers);
		void Destroy();

	private:
		void CalculateDeviceScore();

	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
		VkPhysicalDeviceFeatures2 m_DeviceFeatures;
		VkPhysicalDeviceExtendedDynamicState3FeaturesEXT m_ExtendedDynamicStateDeviceFeatures;
		VkPhysicalDeviceVulkan13Features m_DeviceFeatures13;
		VkDevice m_Device = VK_NULL_HANDLE;
		uint32_t m_DeviceScore = 0;

		uint32_t m_QueueFamilyIndex = std::numeric_limits<uint32_t>::max();
		VkQueue m_Queue = VK_NULL_HANDLE;
	};

}
