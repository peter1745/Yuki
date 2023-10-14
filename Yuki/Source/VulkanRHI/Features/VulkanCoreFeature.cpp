#include "VulkanCoreFeature.hpp"

namespace Yuki {

	VulkanCoreFeature::VulkanCoreFeature()
		: VulkanFeature(RHI::RendererFeature::Core)
	{
	}

	const DynamicArray<std::string_view> Yuki::VulkanCoreFeature::GetRequiredExtensions() const
	{
		return {
			VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME,
		};
	}

	void VulkanCoreFeature::PopulatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2& deviceFeatures)
	{
		m_Vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		m_Vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		m_Vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
		m_Vulkan12Features.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
		m_Vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
		m_Vulkan12Features.runtimeDescriptorArray = VK_TRUE;
		m_Vulkan12Features.scalarBlockLayout = VK_TRUE;
		m_Vulkan12Features.imagelessFramebuffer = VK_TRUE;
		m_Vulkan12Features.timelineSemaphore = VK_TRUE;
		m_Vulkan12Features.bufferDeviceAddress = VK_TRUE;
		AddToPNext(deviceFeatures, m_Vulkan12Features);

		m_Vulkan13Features.synchronization2 = VK_TRUE;
		m_Vulkan13Features.dynamicRendering = VK_TRUE;
		AddToPNext(deviceFeatures, m_Vulkan13Features);

		m_HostImageCopyFeatures.hostImageCopy = VK_TRUE;
		AddToPNext(deviceFeatures, m_HostImageCopyFeatures);
	}

}
