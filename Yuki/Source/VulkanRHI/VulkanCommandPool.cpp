#include "VulkanCommandPool.hpp"
#include "VulkanRenderDevice.hpp"

namespace Yuki::RHI {

	CommandPoolRH VulkanRenderDevice::CommandPoolCreate(QueueRH InQueue)
	{
		auto[Handle, Pool] = m_CommandPools.Acquire();
		const auto& Queue = m_Queues[InQueue];

		VkCommandPoolCreateInfo PoolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.queueFamilyIndex = Queue.Family
		};
		YUKI_VERIFY(vkCreateCommandPool(m_Device, &PoolInfo, nullptr, &Pool.Handle) == VK_SUCCESS);

		return Handle;
	}

	void VulkanRenderDevice::CommandPoolReset(CommandPoolRH InPool)
	{
		auto& Pool = m_CommandPools[InPool];
		YUKI_VERIFY(vkResetCommandPool(m_Device, Pool.Handle, 0) == VK_SUCCESS);
		Pool.NextList = 0;
	}

	CommandListRH VulkanRenderDevice::CommandPoolNewList(CommandPoolRH InPool)
	{
		auto& Pool = m_CommandPools[InPool];

		if (Pool.NextList >= Pool.AllocatedLists.size())
		{
			// TODO(Peter): Maybe consider growing by 50% each time?

			auto [Handle, List] = m_CommandLists.Acquire();

			VkCommandBufferAllocateInfo AllocInfo =
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = nullptr,
				.commandPool = Pool.Handle,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};
			YUKI_VERIFY(vkAllocateCommandBuffers(m_Device, &AllocInfo, &List.Handle) == VK_SUCCESS);

			Pool.AllocatedLists.push_back(Handle);
		}

		return Pool.AllocatedLists[Pool.NextList++];
	}

	void VulkanRenderDevice::CommandPoolDestroy(CommandPoolRH InPool)
	{
		auto& Pool = m_CommandPools[InPool];
		vkDestroyCommandPool(m_Device, Pool.Handle, nullptr);
		m_CommandPools.Return(InPool);
	}

	void VulkanRenderDevice::CommandListBegin(CommandListRH InList)
	{
		auto& List = m_CommandLists[InList];

		VkCommandBufferBeginInfo BeginInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr,
		};
		YUKI_VERIFY(vkBeginCommandBuffer(List.Handle, &BeginInfo) == VK_SUCCESS);
	}

	void VulkanRenderDevice::CommandListImageBarrier(CommandListRH InList, ImageBarrier InBarrier)
	{
		YUKI_VERIFY(InBarrier.Images.Count() == InBarrier.Layouts.Count());

		auto& List = m_CommandLists[InList];

		DynamicArray<VkImageMemoryBarrier2> Barriers(InBarrier.Images.Count(), { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 });

		for (size_t Index = 0; Index < InBarrier.Images.Count(); Index++)
		{
			auto& Image = m_Images[InBarrier.Images[Index]];

			auto NewLayout = ImageLayoutToVkImageLayout(InBarrier.Layouts[Index]);

			Image.OldLayout = Image.Layout;

			Barriers[Index].srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			Barriers[Index].srcAccessMask = 0;
			Barriers[Index].dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			Barriers[Index].dstAccessMask = 0;
			Barriers[Index].oldLayout = Image.OldLayout;
			Barriers[Index].newLayout = NewLayout;
			Barriers[Index].image = Image.Handle;
			Barriers[Index].subresourceRange =
			{
				.aspectMask = Image.AspectMask,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			};

			Image.Layout = NewLayout;
		}

		VkDependencyInfo DependencyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.imageMemoryBarrierCount = Cast<uint32_t>(Barriers.size()),
			.pImageMemoryBarriers = Barriers.data(),
		};
		vkCmdPipelineBarrier2(List.Handle, &DependencyInfo);
	}

	void VulkanRenderDevice::CommandListBeginRendering(CommandListRH InList, RenderTarget InRenderTarget)
	{
		auto& List = m_CommandLists[InList];
		
		DynamicArray<VkRenderingAttachmentInfo> ColorAttachmentInfos(InRenderTarget.ColorAttachments.Count(), { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO });

		VkRect2D RenderArea;

		for (size_t Index = 0; Index < InRenderTarget.ColorAttachments.Count(); Index++)
		{
			const auto& Attachment = InRenderTarget.ColorAttachments[Index];
			const auto& ImageView = m_ImageViews[Attachment.ImageView];
			const auto& Image = m_Images[ImageView.Image];

			RenderArea.offset = { 0, 0 };
			RenderArea.extent = { Image.Width, Image.Height };

			VkAttachmentLoadOp LoadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
			switch (Attachment.LoadOp)
			{
			case AttachmentLoadOp::Load:
			{
				LoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			}
			case AttachmentLoadOp::Clear:
			{
				LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			}
			case AttachmentLoadOp::DontCare:
			{
				LoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
			}
			}

			VkAttachmentStoreOp StoreOp = VK_ATTACHMENT_STORE_OP_NONE;
			switch (Attachment.StoreOp)
			{
			case AttachmentStoreOp::Store:
			{
				StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
				break;
			}
			case AttachmentStoreOp::DontCare:
			{
				StoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				break;
			}
			}

			ColorAttachmentInfos[Index].imageView = ImageView.Handle;
			ColorAttachmentInfos[Index].imageLayout = Image.Layout;
			ColorAttachmentInfos[Index].loadOp = LoadOp;
			ColorAttachmentInfos[Index].storeOp = StoreOp;
			ColorAttachmentInfos[Index].clearValue.color.float32[0] = 1.0f;
			ColorAttachmentInfos[Index].clearValue.color.float32[1] = 0.0f;
			ColorAttachmentInfos[Index].clearValue.color.float32[2] = 0.0f;
			ColorAttachmentInfos[Index].clearValue.color.float32[3] = 1.0f;
		}

		VkRenderingInfo RenderingInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderArea = RenderArea,
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = Cast<uint32_t>(ColorAttachmentInfos.size()),
			.pColorAttachments = ColorAttachmentInfos.data(),
			//.pDepthAttachment,
			//.pStencilAttachment,
		};

		vkCmdBeginRendering(List.Handle, &RenderingInfo);
	}

	void VulkanRenderDevice::CommandListEndRendering(CommandListRH InList)
	{
		auto& List = m_CommandLists[InList];
		vkCmdEndRendering(List.Handle);
	}

	void VulkanRenderDevice::CommandListEnd(CommandListRH InList)
	{
		auto& List = m_CommandLists[InList];
		YUKI_VERIFY(vkEndCommandBuffer(List.Handle) == VK_SUCCESS);
	}

}
