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
		AddToPNext(InDeviceFeatures, m_AccelerationStructureFeatures);
		AddToPNext(InDeviceFeatures, m_RayTracingPipelineFeatures);
	}

}
