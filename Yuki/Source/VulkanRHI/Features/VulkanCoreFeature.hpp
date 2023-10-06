#pragma once

#include "VulkanFeature.hpp"

namespace Yuki {

	class VulkanCoreFeature : public VulkanFeature
	{
	public:
		VulkanCoreFeature();

		const DynamicArray<std::string_view> GetRequiredExtensions() const override;
		void PopulatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2& InDeviceFeatures) override;

	public:
		YUKI_VULKAN_FEATURE_IMPL(Core)

	private:
		VkPhysicalDeviceVulkan13Features m_Vulkan13Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		VkPhysicalDeviceVulkan12Features m_Vulkan12Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		VkPhysicalDeviceMaintenance5FeaturesKHR m_Maintenance5Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR };
	};

}