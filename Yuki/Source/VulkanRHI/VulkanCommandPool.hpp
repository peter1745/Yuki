#pragma once

#include "Engine/RHI/RenderHandles.hpp"

#include "VulkanInclude.hpp"

namespace Yuki::RHI {

	struct VulkanCommandPool
	{
		VkCommandPool Handle = VK_NULL_HANDLE;
		DynamicArray<CommandListRH> AllocatedLists;
		size_t NextList = 0;
	};

	struct VulkanCommandList
	{
		VkCommandBuffer Handle = VK_NULL_HANDLE;
	};

}
