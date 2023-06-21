#pragma once

#include "Rendering/RHI/Sampler.hpp"

#include "VulkanInclude.hpp"

namespace Yuki {

	class VulkanRenderContext;

	class VulkanSampler : public Sampler
	{
	public:
		VkSampler GetVkSampler() const { return m_Sampler; }

	private:
		VulkanSampler(VulkanRenderContext* InContext);

	private:
		VulkanRenderContext* m_Context = nullptr;
		VkSampler m_Sampler = VK_NULL_HANDLE;

	private:
		friend class VulkanRenderContext;
	};

}
