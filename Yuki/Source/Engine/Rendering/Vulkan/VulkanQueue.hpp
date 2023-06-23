#pragma once

#include "VulkanInclude.hpp"

namespace Yuki {

	struct VulkanQueue
	{
		VkQueue Queue = VK_NULL_HANDLE;
		uint32_t FamilyIndex = UINT32_MAX;
	};

}
