#pragma once

#include "Rendering/RHI.hpp"

#include "VulkanInclude.hpp"

namespace Yuki {

	struct VulkanDescriptorSetLayout
	{
		VkDescriptorSetLayout Handle;
	};

	struct VulkanDescriptorPool
	{
		VkDescriptorPool Handle;
		DynamicArray<DescriptorSet> Sets;
	};

	struct VulkanDescriptorSet
	{
		DescriptorSetLayout Layout;
		VkDescriptorSet Handle;
	};

}
