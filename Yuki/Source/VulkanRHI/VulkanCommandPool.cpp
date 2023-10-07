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
			ColorAttachmentInfos[Index].clearValue.color.float32[0] = 0.1f;
			ColorAttachmentInfos[Index].clearValue.color.float32[1] = 0.1f;
			ColorAttachmentInfos[Index].clearValue.color.float32[2] = 0.1f;
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

	void VulkanRenderDevice::CommandListCopyBuffer(CommandListRH InList, BufferRH InDest, BufferRH InSrc)
	{
		auto& List = m_CommandLists[InList];
		
		auto& Dest = m_Buffers[InDest];
		auto& Src = m_Buffers[InSrc];

		uint64_t Size = std::min(Dest.Size, Src.Size);

		VkBufferCopy2 Region =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
			.pNext = nullptr,
			.srcOffset = 0,
			.dstOffset = 0,
			.size = Size,
		};

		VkCopyBufferInfo2 CopyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
			.pNext = nullptr,
			.srcBuffer = Src.Handle,
			.dstBuffer = Dest.Handle,
			.regionCount = 1,
			.pRegions = &Region,
		};
		vkCmdCopyBuffer2(List.Handle, &CopyInfo);
	}

	static VkShaderStageFlags ShaderStagesToVkShaderStageFlags(ShaderStage InStages)
	{
		VkShaderStageFlags Result = 0;

		if (InStages & ShaderStage::Vertex) Result |= VK_SHADER_STAGE_VERTEX_BIT;
		if (InStages & ShaderStage::Fragment) Result |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if (InStages & ShaderStage::RayGeneration) Result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		if (InStages & ShaderStage::RayMiss) Result |= VK_SHADER_STAGE_MISS_BIT_KHR;
		if (InStages & ShaderStage::RayClosestHit) Result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		return Result;
	}

	void VulkanRenderDevice::CommandListPushConstants(CommandListRH InList, PipelineRH InPipeline, ShaderStage InStages, const void* InData, uint32_t InDataSize)
	{
		auto& List = m_CommandLists[InList];
		const auto& Pipeline = m_Pipelines[InPipeline];
		vkCmdPushConstants(List.Handle, Pipeline.Layout, VK_SHADER_STAGE_ALL, 0, InDataSize, InData);
	}

	void VulkanRenderDevice::CommandListPushConstants(CommandListRH InList, RayTracingPipelineRH InPipeline, ShaderStage InStages, const void* InData, uint32_t InDataSize)
	{
		auto& List = m_CommandLists[InList];
		const auto& Pipeline = m_RayTracingPipelines[InPipeline];
		vkCmdPushConstants(List.Handle, Pipeline.Layout, VK_SHADER_STAGE_ALL, 0, InDataSize, InData);
	}

	void VulkanRenderDevice::CommandListBindDescriptorSets(CommandListRH InList, PipelineRH InPipeline, Span<DescriptorSetRH> InDescriptorSets)
	{
		auto& List = m_CommandLists[InList];
		const auto& Pipeline = m_Pipelines[InPipeline];

		DynamicArray<VkDescriptorSet> Sets(InDescriptorSets.Count());
		for (size_t Index = 0; Index < InDescriptorSets.Count(); Index++)
			Sets[Index] = m_DescriptorSets[InDescriptorSets[Index]].Handle;

		vkCmdBindDescriptorSets(List.Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.Layout, 0, Cast<uint32_t>(InDescriptorSets.Count()), Sets.data(), 0, nullptr);
	}

	void VulkanRenderDevice::CommandListBindDescriptorSets(CommandListRH InList, RayTracingPipelineRH InPipeline, Span<DescriptorSetRH> InDescriptorSets)
	{
		auto& List = m_CommandLists[InList];
		const auto& Pipeline = m_RayTracingPipelines[InPipeline];

		DynamicArray<VkDescriptorSet> Sets(InDescriptorSets.Count());
		for (size_t Index = 0; Index < InDescriptorSets.Count(); Index++)
			Sets[Index] = m_DescriptorSets[InDescriptorSets[Index]].Handle;

		vkCmdBindDescriptorSets(List.Handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipeline.Layout, 0, InDescriptorSets.Count(), Sets.data(), 0, nullptr);
	}

	void VulkanRenderDevice::CommandListBindPipeline(CommandListRH InList, PipelineRH InPipeline)
	{
		auto& List = m_CommandLists[InList];
		const auto& Pipeline = m_Pipelines[InPipeline];
		vkCmdBindPipeline(List.Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.Handle);
	}

	void VulkanRenderDevice::CommandListBindPipeline(CommandListRH InList, RayTracingPipelineRH InPipeline)
	{
		auto& List = m_CommandLists[InList];
		const auto& Pipeline = m_RayTracingPipelines[InPipeline];
		vkCmdBindPipeline(List.Handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Pipeline.Handle);
	}

	void VulkanRenderDevice::CommandListBindIndexBuffer(CommandListRH InList, BufferRH InBuffer)
	{
		auto& List = m_CommandLists[InList];
		const auto& Buffer = m_Buffers[InBuffer];
		vkCmdBindIndexBuffer(List.Handle, Buffer.Handle, 0, VK_INDEX_TYPE_UINT32);
	}

	void VulkanRenderDevice::CommandListSetViewport(CommandListRH InList, Viewport InViewport)
	{
		auto& List = m_CommandLists[InList];
		VkViewport Viewport =
		{
			.x = InViewport.X,
			.y = InViewport.Y,
			.width = InViewport.Width,
			.height = InViewport.Height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};
		vkCmdSetViewport(List.Handle, 0, 1, &Viewport);

		VkRect2D Scissor =
		{
			.offset = { 0, 0 },
			.extent = { Cast<uint32_t>(InViewport.Width), Cast<uint32_t>(InViewport.Height) }
		};
		vkCmdSetScissor(List.Handle, 0, 1, &Scissor);
	}

	void VulkanRenderDevice::CommandListDrawIndexed(CommandListRH InList, uint32_t InIndexCount, uint32_t InIndexOffset, uint32_t InInstanceIndex)
	{
		auto& List = m_CommandLists[InList];
		vkCmdDrawIndexed(List.Handle, InIndexCount, 1, 0, 0, InInstanceIndex);
	}

	void VulkanRenderDevice::CommandListTraceRay(CommandListRH InList, RayTracingPipelineRH InPipeline, uint32_t InWidth, uint32_t InHeight)
	{
		auto& List = m_CommandLists[InList];
		const auto& Pipeline = m_RayTracingPipelines[InPipeline];
		vkCmdTraceRaysKHR(List.Handle, &Pipeline.RayGenRegion, &Pipeline.MissGenRegion, &Pipeline.ClosestHitGenRegion, &Pipeline.CallableGenRegion, InWidth, InHeight, 1);
	}

	void VulkanRenderDevice::CommandListEnd(CommandListRH InList)
	{
		auto& List = m_CommandLists[InList];
		YUKI_VERIFY(vkEndCommandBuffer(List.Handle) == VK_SUCCESS);
	}

}
