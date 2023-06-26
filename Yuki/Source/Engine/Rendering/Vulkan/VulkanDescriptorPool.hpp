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
		DynamicArray<DescriptorSetHandle> Sets;
	};

	struct VulkanDescriptorSet
	{
		DescriptorSetLayoutHandle Layout;
		VkDescriptorSet Handle;
	};

}
