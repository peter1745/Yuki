#include "VulkanRaytracingFeature.hpp"

namespace Yuki {

	VulkanRaytracingFeature::VulkanRaytracingFeature()
		: VulkanFeature(RHI::RendererFeature::RayTracing)
	{
	}

	const DynamicArray<std::string_view> VulkanRaytracingFeature::GetRequiredExtensions() const
	{
		return {
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
		};
	}

	void VulkanRaytracingFeature::PopulatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2& deviceFeatures)
	{
		m_AccelerationStructureFeatures.accelerationStructure = VK_TRUE;
		AddToPNext(deviceFeatures, m_AccelerationStructureFeatures);

		m_RayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
		AddToPNext(deviceFeatures, m_RayTracingPipelineFeatures);
	}

	void VulkanRaytracingFeature::PopulateProperties(VkPhysicalDeviceProperties2& properties)
	{
		AddToPNext(properties, m_RayTracingProperties);
		AddToPNext(properties, m_AccelerationStructureProperties);
	}

}
