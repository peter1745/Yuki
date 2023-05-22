#include "VulkanRenderInterface.hpp"

#include "Vulkan.hpp"

namespace Yuki {

	void VulkanRenderInterface::BeginRendering(CommandBuffer InCmdBuffer)
	{
		VkRenderingInfo renderingInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO
		};

		vkCmdBeginRendering(InCmdBuffer.As<VkCommandBuffer>(), &renderingInfo);
	}

	void VulkanRenderInterface::EndRendering(CommandBuffer InCmdBuffer)
	{
	}

}
