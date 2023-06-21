#include "VulkanSampler.hpp"
#include "VulkanRenderContext.hpp"

namespace Yuki {

	VulkanSampler::VulkanSampler(VulkanRenderContext* InContext)
		: m_Context(InContext)
	{
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

		vkCreateSampler(InContext->GetDevice(), &samplerInfo, nullptr, &m_Sampler);
	}

	VulkanSampler::~VulkanSampler()
	{
		vkDestroySampler(m_Context->GetDevice(), m_Sampler, nullptr);
	}

}
