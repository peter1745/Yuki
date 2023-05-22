#pragma once

#include "Vulkan.hpp"

namespace Yuki {

	class VulkanQueue
	{
	public:
		uint32_t GetFamilyIndex() const { return m_QueueFamily; }

	private:
		VkQueue m_Queue = VK_NULL_HANDLE;
		uint32_t m_QueueFamily = 0;

		friend class VulkanRenderContext;
	};

}
