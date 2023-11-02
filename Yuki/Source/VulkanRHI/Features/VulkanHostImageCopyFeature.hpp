#pragma once

#include "VulkanFeature.hpp"

namespace Yuki {

	class VulkanHostImageCopyFeature : public VulkanFeature
	{
	public:
		VulkanHostImageCopyFeature();

		const DynamicArray<std::string_view> GetRequiredExtensions() const override;
		void PopulatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2& deviceFeatures) override;

	public:
		YUKI_VULKAN_FEATURE_IMPL(HostImageCopy)

	private:
		VkPhysicalDeviceHostImageCopyFeaturesEXT m_HostImageCopyFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT };
	};

}