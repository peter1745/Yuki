#include "VulkanRHI.hpp"

namespace Yuki::RHI {

	CommandPool CommandPool::Create(Context InContext, QueueRH InQueue)
	{
		auto Pool = new Impl();
		Pool->Ctx = InContext;

		VkCommandPoolCreateInfo PoolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.queueFamilyIndex = InQueue->Family
		};
		YUKI_VERIFY(vkCreateCommandPool(InContext->Device, &PoolInfo, nullptr, &Pool->Handle) == VK_SUCCESS);

		return { Pool };
	}

	void CommandPool::Destroy()
	{
		for (auto List : m_Impl->AllocatedLists)
			delete List.m_Impl;

		vkDestroyCommandPool(m_Impl->Ctx->Device, m_Impl->Handle, nullptr);
		delete m_Impl;
	}

	void CommandPool::Reset()
	{
		YUKI_VERIFY(vkResetCommandPool(m_Impl->Ctx->Device, m_Impl->Handle, 0) == VK_SUCCESS);
		m_Impl->NextList = 0;
	}

	CommandList CommandPool::NewList()
	{
		if (m_Impl->NextList >= m_Impl->AllocatedLists.size())
		{
			// TODO(Peter): Maybe consider growing by 50% each time?

			auto List = new CommandList::Impl();

			VkCommandBufferAllocateInfo AllocInfo =
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = nullptr,
				.commandPool = m_Impl->Handle,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};
			YUKI_VERIFY(vkAllocateCommandBuffers(m_Impl->Ctx->Device, &AllocInfo, &List->Handle) == VK_SUCCESS);

			m_Impl->AllocatedLists.push_back({ List });
		}

		return m_Impl->AllocatedLists[m_Impl->NextList++];
	}

	void CommandList::Begin()
	{
		VkCommandBufferBeginInfo BeginInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr,
		};
		YUKI_VERIFY(vkBeginCommandBuffer(m_Impl->Handle, &BeginInfo) == VK_SUCCESS);
	}

	void CommandList::ImageBarrier(RHI::ImageBarrier InBarrier)
	{
		YUKI_VERIFY(InBarrier.Images.Count() == InBarrier.Layouts.Count());

		DynamicArray<VkImageMemoryBarrier2> Barriers(InBarrier.Images.Count(), { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 });

		for (size_t Index = 0; Index < InBarrier.Images.Count(); Index++)
		{
			auto Image = InBarrier.Images[Index];
			auto NewLayout = Image::Impl::ImageLayoutToVkImageLayout(InBarrier.Layouts[Index]);

			Image->OldLayout = Image->Layout;

			Barriers[Index].srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			Barriers[Index].srcAccessMask = 0;
			Barriers[Index].dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			Barriers[Index].dstAccessMask = 0;
			Barriers[Index].oldLayout = Image->OldLayout;
			Barriers[Index].newLayout = NewLayout;
			Barriers[Index].image = Image->Handle;
			Barriers[Index].subresourceRange =
			{
				.aspectMask = Image->AspectMask,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			};

			Image->Layout = NewLayout;
		}

		VkDependencyInfo DependencyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = nullptr,
			.imageMemoryBarrierCount = Cast<uint32_t>(Barriers.size()),
			.pImageMemoryBarriers = Barriers.data(),
		};
		vkCmdPipelineBarrier2(m_Impl->Handle, &DependencyInfo);
	}

	void CommandList::BeginRendering(RenderTarget InRenderTarget)
	{
		DynamicArray<VkRenderingAttachmentInfo> ColorAttachmentInfos(InRenderTarget.ColorAttachments.Count(), { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO });

		VkRect2D RenderArea;

		for (size_t Index = 0; Index < InRenderTarget.ColorAttachments.Count(); Index++)
		{
			const auto& Attachment = InRenderTarget.ColorAttachments[Index];

			RenderArea.offset = { 0, 0 };
			RenderArea.extent = { Attachment.ImageView->Image->Width, Attachment.ImageView->Image->Height };

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

			ColorAttachmentInfos[Index].imageView = Attachment.ImageView->Handle;
			ColorAttachmentInfos[Index].imageLayout = Attachment.ImageView->Image->Layout;
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

		vkCmdBeginRendering(m_Impl->Handle, &RenderingInfo);
	}

	void CommandList::EndRendering()
	{
		vkCmdEndRendering(m_Impl->Handle);
	}

	void CommandList::CopyBuffer(BufferRH InDest, BufferRH InSrc)
	{
		uint64_t Size = std::min(InDest->Size, InSrc->Size);

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
			.srcBuffer = InSrc->Handle,
			.dstBuffer = InDest->Handle,
			.regionCount = 1,
			.pRegions = &Region,
		};
		vkCmdCopyBuffer2(m_Impl->Handle, &CopyInfo);
	}

	/*static VkShaderStageFlags ShaderStagesToVkShaderStageFlags(ShaderStage InStages)
	{
		VkShaderStageFlags Result = 0;

		if (InStages & ShaderStage::Vertex) Result |= VK_SHADER_STAGE_VERTEX_BIT;
		if (InStages & ShaderStage::Fragment) Result |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if (InStages & ShaderStage::RayGeneration) Result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		if (InStages & ShaderStage::RayMiss) Result |= VK_SHADER_STAGE_MISS_BIT_KHR;
		if (InStages & ShaderStage::RayClosestHit) Result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		return Result;
	}*/

	void CommandList::PushConstants(PipelineRH InPipeline, ShaderStage InStages, const void* InData, uint32_t InDataSize)
	{
		vkCmdPushConstants(m_Impl->Handle, InPipeline->Layout, VK_SHADER_STAGE_ALL, 0, InDataSize, InData);
	}

	void CommandList::PushConstants(RayTracingPipelineRH InPipeline, ShaderStage InStages, const void* InData, uint32_t InDataSize)
	{
		vkCmdPushConstants(m_Impl->Handle, InPipeline->Layout, VK_SHADER_STAGE_ALL, 0, InDataSize, InData);
	}

	void CommandList::BindDescriptorSets(PipelineRH InPipeline, Span<DescriptorSetRH> InDescriptorSets)
	{
		DynamicArray<VkDescriptorSet> Sets(InDescriptorSets.Count());
		for (size_t Index = 0; Index < InDescriptorSets.Count(); Index++)
			Sets[Index] = InDescriptorSets[Index]->Handle;

		vkCmdBindDescriptorSets(m_Impl->Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, InPipeline->Layout, 0, Cast<uint32_t>(InDescriptorSets.Count()), Sets.data(), 0, nullptr);
	}

	void CommandList::BindDescriptorSets(RayTracingPipelineRH InPipeline, Span<DescriptorSetRH> InDescriptorSets)
	{
		DynamicArray<VkDescriptorSet> Sets(InDescriptorSets.Count());
		for (size_t Index = 0; Index < InDescriptorSets.Count(); Index++)
			Sets[Index] = InDescriptorSets[Index]->Handle;

		vkCmdBindDescriptorSets(m_Impl->Handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, InPipeline->Layout, 0, Cast<uint32_t>(InDescriptorSets.Count()), Sets.data(), 0, nullptr);
	}

	void CommandList::BindPipeline(PipelineRH InPipeline)
	{
		vkCmdBindPipeline(m_Impl->Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, InPipeline->Handle);
	}

	void CommandList::BindPipeline(RayTracingPipelineRH InPipeline)
	{
		vkCmdBindPipeline(m_Impl->Handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, InPipeline->Handle);
	}

	void CommandList::BindIndexBuffer(BufferRH InBuffer)
	{
		vkCmdBindIndexBuffer(m_Impl->Handle, InBuffer->Handle, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandList::SetViewport(Viewport InViewport)
	{
		VkViewport Viewport =
		{
			.x = InViewport.X,
			.y = InViewport.Y,
			.width = InViewport.Width,
			.height = InViewport.Height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};
		vkCmdSetViewport(m_Impl->Handle, 0, 1, &Viewport);

		VkRect2D Scissor =
		{
			.offset = { 0, 0 },
			.extent = { Cast<uint32_t>(InViewport.Width), Cast<uint32_t>(InViewport.Height) }
		};
		vkCmdSetScissor(m_Impl->Handle, 0, 1, &Scissor);
	}

	void CommandList::DrawIndexed(uint32_t InIndexCount, uint32_t InIndexOffset, uint32_t InInstanceIndex)
	{
		vkCmdDrawIndexed(m_Impl->Handle, InIndexCount, 1, 0, 0, InInstanceIndex);
	}

	void CommandList::TraceRay(RayTracingPipelineRH InPipeline, uint32_t InWidth, uint32_t InHeight)
	{
		vkCmdTraceRaysKHR(m_Impl->Handle, &InPipeline->RayGenRegion, &InPipeline->MissGenRegion, &InPipeline->ClosestHitGenRegion, &InPipeline->CallableGenRegion, InWidth, InHeight, 1);
	}

	void CommandList::End()
	{
		YUKI_VERIFY(vkEndCommandBuffer(m_Impl->Handle) == VK_SUCCESS);
	}

}
