#include "VulkanHostImageCopyFeature.hpp"

namespace Yuki {

	VulkanHostImageCopyFeature::VulkanHostImageCopyFeature()
		: VulkanFeature(RHI::RendererFeature::HostImageCopy)
	{
	}

	const DynamicArray<std::string_view> VulkanHostImageCopyFeature::GetRequiredExtensions() const
	{
		return { VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME };
	}

	void VulkanHostImageCopyFeature::PopulatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2& deviceFeatures)
	{
		m_HostImageCopyFeatures.hostImageCopy = VK_TRUE;
		AddToPNext(deviceFeatures, m_HostImageCopyFeatures);
	}

}
