#pragma once

#include "Vulkan.hpp"

namespace Yuki {

	class VulkanDevice
	{
	public:
		VulkanDevice(VkPhysicalDevice InPhysicalDevice);

		void WaitIdle() const;

		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkDevice GetLogicalDevice() const { return m_Device; }

		uint32_t GetDeviceScore() const { return m_DeviceScore; }

		const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const { return m_PhysicalDeviceProperties; }

	public:
		void CreateLogicalDevice(VkSurfaceKHR InSurface, const List<const char*>& InDeviceLayers);
		void Destroy();

	private:
		void CalculateDeviceScore();

	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
		VkPhysicalDeviceFeatures2 m_DeviceFeatures;
		VkPhysicalDeviceVulkan13Features m_DeviceFeatures13;
		VkDevice m_Device = VK_NULL_HANDLE;
		uint32_t m_DeviceScore = 0;
		VkQueue m_Queue = VK_NULL_HANDLE;
	};

}
