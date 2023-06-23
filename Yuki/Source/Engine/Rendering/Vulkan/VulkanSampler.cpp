#include "VulkanSampler.hpp"
#include "VulkanRenderContext.hpp"

namespace Yuki {

	Sampler VulkanRenderContext::CreateSampler()
	{
		auto[handle, sampler] = m_Samplers.Acquire();

		VkSamplerCreateInfo samplerInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		};
		vkCreateSampler(m_LogicalDevice, &samplerInfo, nullptr, &sampler.Handle);

		return handle;
	}

	void VulkanRenderContext::Destroy(Sampler InSampler)
	{
		auto& sampler = m_Samplers.Get(InSampler);
		vkDestroySampler(m_LogicalDevice, sampler.Handle, nullptr);
		m_Samplers.Return(InSampler);
	}

}
