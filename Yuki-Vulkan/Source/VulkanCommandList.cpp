#include "VulkanRHI.hpp"

#include <Aura/Stack.hpp>

namespace Yuki {

	void CommandList::BeginRendering(Aura::Span<RenderingAttachment> colorAttachments) const
	{
		AuraStackPoint();

		auto attachments = Aura::StackAlloc<VkRenderingAttachmentInfo>(colorAttachments.Count());

		uint32_t renderWidth = ~0u;
		uint32_t renderHeight = ~0u;

		for (uint32_t i = 0; i < colorAttachments.Count(); i++)
		{
			auto imageView = colorAttachments[i].Target;

			if (imageView->Source->Width < renderWidth)
			{
				renderWidth = imageView->Source->Width;
			}

			if (imageView->Source->Height < renderHeight)
			{
				renderHeight = imageView->Source->Height;
			}

			attachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			attachments[i].imageView = imageView->Resource;
			attachments[i].imageLayout = imageView->Source->Layout;
			attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[i].clearValue = {
				.color = {
					.float32 = { 1.0f, 0.0f, 0.0f, 1.0f }
				}
			};
		}

		VkRenderingInfo renderingInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea = {
				{ 0, 0 },
				{ renderWidth, renderHeight }
			},
			.layerCount = 1,
			.colorAttachmentCount = attachments.Count(),
			.pColorAttachments = attachments.Data(),
		};

		vkCmdBeginRendering(m_Impl->Resource, &renderingInfo);
	}

	void CommandList::EndRendering() const
	{
		vkCmdEndRendering(m_Impl->Resource);
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
