#pragma once

#include "Engine/RHI/RenderHandles.hpp"

#include "VulkanInclude.hpp"

namespace Yuki::RHI {

	struct VulkanDescriptorSetLayout
	{
		VkDescriptorSetLayout Handle;
	};

	struct VulkanDescriptorPool
	{
		VkDescriptorPool Handle;
		DynamicArray<DescriptorSetRH> AllocatedSets;
	};

	struct VulkanDescriptorSet
	{
		VkDescriptorSet Handle;
		DescriptorSetLayoutRH Layout;
	};

}
