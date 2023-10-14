#pragma once

#include "VulkanFeature.hpp"

namespace Yuki {

	class VulkanCoreFeature : public VulkanFeature
	{
	public:
		VulkanCoreFeature();

		const DynamicArray<std::string_view> GetRequiredExtensions() const override;
		void PopulatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2& deviceFeatures) override;

	public:
		YUKI_VULKAN_FEATURE_IMPL(Core)

	private:
		VkPhysicalDeviceVulkan13Features m_Vulkan13Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		VkPhysicalDeviceVulkan12Features m_Vulkan12Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		VkPhysicalDeviceHostImageCopyFeaturesEXT m_HostImageCopyFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT };
		VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT m_MutableDescriptorTypeFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT };
	};

}