#include "VulkanRHI.hpp"

namespace Yuki {

	void CommandList::TransitionSwapchain(Swapchain swapchain) const
	{
		VkImageMemoryBarrier2 imageBarrier =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.image = swapchain->Images[swapchain->CurrentImageIndex],
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		VkDependencyInfo dependencyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &imageBarrier,
		};

		vkCmdPipelineBarrier2(m_Impl->Resource, &dependencyInfo);
	}

	CommandPool CommandPool::Create(RHIContext context, Queue queue)
	{
		auto* impl = new Impl();
		impl->Context = context;

		VkCommandPoolCreateInfo poolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.queueFamilyIndex = queue->Family,
		};
		Vulkan::CheckResult(vkCreateCommandPool(context->Device, &poolInfo, nullptr, &impl->Resource));

		return { impl };
	}

	void CommandPool::Reset() const
	{
		vkResetCommandPool(m_Impl->Context->Device, m_Impl->Resource, 0);
		m_Impl->NextList = 0;
	}

	CommandList CommandPool::NewList() const
	{
		if (m_Impl->NextList >= m_Impl->AllocatedLists.size())
		{
			auto* cmd = new CommandList::Impl();

			VkCommandBufferAllocateInfo bufferInfo =
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = nullptr,
				.commandPool = m_Impl->Resource,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};

			Vulkan::CheckResult(vkAllocateCommandBuffers(m_Impl->Context->Device, &bufferInfo, &cmd->Resource));

			m_Impl->AllocatedLists.push_back({ cmd });
		}

		auto cmd = m_Impl->AllocatedLists[m_Impl->NextList++];
		VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, };
		Vulkan::CheckResult(vkBeginCommandBuffer(cmd->Resource, &beginInfo));
		return cmd;
	}

}
