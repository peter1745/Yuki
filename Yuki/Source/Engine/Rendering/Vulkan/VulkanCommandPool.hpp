#pragma once

#include "Rendering/RHI.hpp"

#include "VulkanInclude.hpp"

namespace Yuki {

	struct VulkanCommandPool
	{
		VkCommandPool Pool = VK_NULL_HANDLE;
		DynamicArray<CommandList> AllocatedLists;
		size_t NextList = 0;
	};

	struct VulkanCommandList
	{
		VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
	};

}
