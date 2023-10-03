#pragma once

#include "VulkanFeature.hpp"

namespace Yuki {

	class VulkanRaytracingFeature : public VulkanFeature
	{
	public:
		VulkanRaytracingFeature();

		const DynamicArray<std::string_view> GetRequiredExtensions() const override;
		void PopulatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2& InDeviceFeatures) override;

	public:
		YUKI_VULKAN_FEATURE_IMPL(RayTracing)

	private:
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_RayTracingPipelineFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR m_AccelerationStructureFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
	};

}
