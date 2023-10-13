#include "VulkanRHI.hpp"

namespace Yuki::RHI {

	CommandPool CommandPool::Create(Context context, QueueRH queue, CommandPoolFlag flags)
	{
		auto pool = new Impl();
		pool->Ctx = context;

		VkCommandPoolCreateFlags poolFlags = 0;

		if (flags & CommandPoolFlag::TransientLists)
		{
			poolFlags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		}

		VkCommandPoolCreateInfo poolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = poolFlags,
			.queueFamilyIndex = queue->Family
		};
		YUKI_VK_CHECK(vkCreateCommandPool(context->Device, &poolInfo, nullptr, &pool->Handle));

		return { pool };
	}

	void CommandPool::Destroy()
	{
		for (auto list : m_Impl->AllocatedLists)
			delete list.m_Impl;

		vkDestroyCommandPool(m_Impl->Ctx->Device, m_Impl->Handle, nullptr);
		delete m_Impl;
	}

	void CommandPool::Reset() const
	{
		YUKI_VK_CHECK(vkResetCommandPool(m_Impl->Ctx->Device, m_Impl->Handle, 0));
		m_Impl->NextList = 0;
	}

	CommandList CommandPool::NewList() const
	{
		if (m_Impl->NextList >= m_Impl->AllocatedLists.size())
		{
			// TODO(Peter): Maybe consider growing by 50% each time?

			auto list = new CommandList::Impl();

			VkCommandBufferAllocateInfo allocInfo =
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = nullptr,
				.commandPool = m_Impl->Handle,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1,
			};
			YUKI_VK_CHECK(vkAllocateCommandBuffers(m_Impl->Ctx->Device, &allocInfo, &list->Handle));

			m_Impl->AllocatedLists.push_back({ list });
		}

		return m_Impl->AllocatedLists[m_Impl->NextList++];
	}

	void CommandList::Begin()
	{
		VkCommandBufferBeginInfo beginInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr,
		};
		YUKI_VK_CHECK(vkBeginCommandBuffer(m_Impl->Handle, &beginInfo));
	}

	void CommandList::BeginRendering(RenderTarget renderTarget)
	{
		DynamicArray<VkRenderingAttachmentInfo> colorAttachmentInfos(renderTarget.ColorAttachments.Count(), { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO });

		VkRect2D renderArea = { { 0, 0 }, { 0, 0 } };

		for (size_t i = 0; i < renderTarget.ColorAttachments.Count(); i++)
		{
			const auto& attachment = renderTarget.ColorAttachments[i];

			renderArea.offset = { 0, 0 };
			renderArea.extent = { attachment.ImageView->Image->Width, attachment.ImageView->Image->Height };

			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
			switch (attachment.LoadOp)
			{
			case AttachmentLoadOp::Load:
			{
				loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			}
			case AttachmentLoadOp::Clear:
			{
				loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			}
			case AttachmentLoadOp::DontCare:
			{
				loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
			}
			}

			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_NONE;
			switch (attachment.StoreOp)
			{
			case AttachmentStoreOp::Store:
			{
				storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				break;
			}
			case AttachmentStoreOp::DontCare:
			{
				storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				break;
			}
			}

			colorAttachmentInfos[i].imageView = attachment.ImageView->Handle;
			colorAttachmentInfos[i].imageLayout = attachment.ImageView->Image->Layout;
			colorAttachmentInfos[i].loadOp = loadOp;
			colorAttachmentInfos[i].storeOp = storeOp;
			colorAttachmentInfos[i].clearValue.color.float32[0] = 0.1f;
			colorAttachmentInfos[i].clearValue.color.float32[1] = 0.1f;
			colorAttachmentInfos[i].clearValue.color.float32[2] = 0.1f;
			colorAttachmentInfos[i].clearValue.color.float32[3] = 1.0f;
		}

		VkRenderingInfo renderingInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderArea = renderArea,
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = Cast<uint32_t>(colorAttachmentInfos.size()),
			.pColorAttachments = colorAttachmentInfos.data(),
		};

		vkCmdBeginRendering(m_Impl->Handle, &renderingInfo);
	}

	void CommandList::EndRendering()
	{
		vkCmdEndRendering(m_Impl->Handle);
	}

	void CommandList::CopyBuffer(BufferRH dest, BufferRH src)
	{
		uint64_t size = std::min(dest->Size, src->Size);

		VkBufferCopy2 region =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
			.pNext = nullptr,
			.srcOffset = 0,
			.dstOffset = 0,
			.size = size,
		};

		VkCopyBufferInfo2 copyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
			.pNext = nullptr,
			.srcBuffer = src->Handle,
			.dstBuffer = dest->Handle,
			.regionCount = 1,
			.pRegions = &region,
		};
		vkCmdCopyBuffer2(m_Impl->Handle, &copyInfo);
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

	void CommandList::PushConstants(PipelineLayout layout, ShaderStage stages, const void* data, uint32_t dataSize)
	{
		vkCmdPushConstants(m_Impl->Handle, layout->Handle, VK_SHADER_STAGE_ALL, 0, dataSize, data);
	}

	void CommandList::BindDescriptorSets(PipelineLayout layout, PipelineBindPoint bindPoint, Span<DescriptorSetRH> descriptorSets)
	{
		DynamicArray<VkDescriptorSet> sets(descriptorSets.Count());
		for (size_t i = 0; i < descriptorSets.Count(); i++)
			sets[i] = descriptorSets[i]->Handle;

		VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;

		switch (bindPoint)
		{
		case PipelineBindPoint::Graphics:
			pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			break;
		case PipelineBindPoint::RayTracing:
			pipelineBindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
			break;
		}

		vkCmdBindDescriptorSets(m_Impl->Handle, pipelineBindPoint, layout->Handle, 0, Cast<uint32_t>(descriptorSets.Count()), sets.data(), 0, nullptr);
	}

	void CommandList::BindPipeline(PipelineRH pipeline)
	{
		vkCmdBindPipeline(m_Impl->Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->Handle);
	}

	void CommandList::BindPipeline(RayTracingPipelineRH pipeline)
	{
		vkCmdBindPipeline(m_Impl->Handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->Handle);
	}

	void CommandList::BindIndexBuffer(BufferRH buffer)
	{
		vkCmdBindIndexBuffer(m_Impl->Handle, buffer->Handle, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandList::SetViewport(Viewport viewport)
	{
		VkViewport v =
		{
			.x = viewport.X,
			.y = viewport.Y,
			.width = viewport.Width,
			.height = viewport.Height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};
		vkCmdSetViewport(m_Impl->Handle, 0, 1, &v);

		VkRect2D scissor =
		{
			.offset = { 0, 0 },
			.extent = { Cast<uint32_t>(viewport.Width), Cast<uint32_t>(viewport.Height) }
		};
		vkCmdSetScissor(m_Impl->Handle, 0, 1, &scissor);
	}

	void CommandList::DrawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t instanceIndex)
	{
		vkCmdDrawIndexed(m_Impl->Handle, indexCount, 1, 0, 0, instanceIndex);
	}

	void CommandList::TraceRays(RayTracingPipelineRH pipeline, uint32_t width, uint32_t height)
	{
		vkCmdTraceRaysKHR(m_Impl->Handle, &pipeline->RayGenRegion, &pipeline->MissGenRegion, &pipeline->ClosestHitGenRegion, &pipeline->CallableGenRegion, width, height, 1);
	}

	void CommandList::End()
	{
		YUKI_VK_CHECK(vkEndCommandBuffer(m_Impl->Handle));
	}

}
