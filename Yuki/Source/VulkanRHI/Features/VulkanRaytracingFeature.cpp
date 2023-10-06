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

	void VulkanRaytracingFeature::PopulatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2& InDeviceFeatures)
	{
		m_AccelerationStructureFeatures.accelerationStructure = VK_TRUE;
		AddToPNext(InDeviceFeatures, m_AccelerationStructureFeatures);

		m_RayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
		AddToPNext(InDeviceFeatures, m_RayTracingPipelineFeatures);
	}

	void VulkanRaytracingFeature::PopulateProperties(VkPhysicalDeviceProperties2& InProperties)
	{
		AddToPNext(InProperties, m_RayTracingProperties);
		AddToPNext(InProperties, m_AccelerationStructureProperties);
	}

}
