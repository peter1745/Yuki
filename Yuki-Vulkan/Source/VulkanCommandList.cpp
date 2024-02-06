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
					.float32 = { 0.1f, 0.1f, 0.1f, 1.0f }
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

	void CommandList::SetViewports(Aura::Span<Viewport> viewports) const
	{
		AuraStackPoint();

		auto viewportList = Aura::StackAlloc<VkViewport>(viewports.Count());
		auto scissorList = Aura::StackAlloc<VkRect2D>(viewports.Count());
		for (uint32_t i = 0; i < viewports.Count(); i++)
		{
			viewportList[i].x = 0.0f;
			viewportList[i].y = 0.0f;
			viewportList[i].width = static_cast<float32_t>(viewports[i].Width);
			viewportList[i].height = static_cast<float32_t>(viewports[i].Height);
			viewportList[i].minDepth = 0.0f;
			viewportList[i].maxDepth = 1.0f;
			
			scissorList[i].offset = { 0, 0 };
			scissorList[i].extent = { viewports[i].Width, viewports[i].Height };
		}

		vkCmdSetViewportWithCount(m_Impl->Resource, viewportList.Count(), viewportList.Data());
		vkCmdSetScissorWithCount(m_Impl->Resource, scissorList.Count(), scissorList.Data());
	}

	void CommandList::BindPipeline(GraphicsPipeline pipeline) const
	{
		vkCmdBindPipeline(m_Impl->Resource, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->Resource);
	}

	void CommandList::TransitionImage(Image image, ImageLayout layout) const
	{
		auto newLayout = ImageLayoutToVkImageLayout(layout);

		if (image->Layout == newLayout)
		{
			return;
		}

		image->OldLayout = image->Layout;
		image->Layout = newLayout;

		VkImageMemoryBarrier2 imageBarrier =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			.dstAccessMask = 0,
			.oldLayout = image->OldLayout,
			.newLayout = image->Layout,
			.image = image->Allocation.Resource,
			.subresourceRange = {
				.aspectMask = image->AspectFlags,
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

	void CommandList::BlitImage(Image dest, Image src) const
	{
		VkImageBlit2 imageBlit =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
			.srcSubresource = {
				.aspectMask = src->AspectFlags,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffsets = {
				{ 0, 0, 0 },
				{ src->Width, src->Height, 1 },
			},
			.dstSubresource = {
				.aspectMask = dest->AspectFlags,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffsets = {
				{ 0, 0, 0 },
				{ dest->Width, dest->Height, 1 },
			},
		};

		VkBlitImageInfo2 blitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
			.srcImage = src->Allocation.Resource,
			.srcImageLayout = src->Layout,
			.dstImage = dest->Allocation.Resource,
			.dstImageLayout = dest->Layout,
			.regionCount = 1,
			.pRegions = &imageBlit,
			.filter = VK_FILTER_NEAREST,
		};

		vkCmdBlitImage2(m_Impl->Resource, &blitInfo);
	}

	void CommandList::BindDescriptorHeap(DescriptorHeap heap, GraphicsPipeline pipeline) const
	{
		vkCmdBindDescriptorSets(
			m_Impl->Resource,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->Layout,
			0,
			1,
			&heap->Set,
			0,
			nullptr
		);
	}

	void CommandList::BindVertexBuffer(Buffer buffer, uint32_t stride) const
	{
		VkDeviceSize offset = 0;
		VkDeviceSize size = buffer->Size;
		VkDeviceSize stride64 = stride;
		vkCmdBindVertexBuffers2(m_Impl->Resource, 0, 1, &buffer->Allocation.Resource, &offset, &size, &stride64);
	}

	void CommandList::BindIndexBuffer(Buffer buffer) const
	{
		vkCmdBindIndexBuffer(m_Impl->Resource, buffer->Allocation.Resource, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandList::CopyBuffer(Buffer dest, Buffer src, uint32_t size, uint32_t srcOffset, uint32_t destOffset) const
	{
		VkBufferCopy2 bufferCopy =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
			.srcOffset = srcOffset,
			.dstOffset = destOffset,
			.size = size,
		};

		VkCopyBufferInfo2 copyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
			.srcBuffer = src->Allocation.Resource,
			.dstBuffer = dest->Allocation.Resource,
			.regionCount = 1,
			.pRegions = &bufferCopy,
		};

		vkCmdCopyBuffer2(m_Impl->Resource, &copyInfo);
	}

	void CommandList::CopyBufferToImage(Image dest, Buffer src, uint32_t size, uint32_t srcOffset) const
	{
		VkBufferImageCopy2 bufferImageCopy =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
			.bufferOffset = srcOffset,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = dest->AspectFlags,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { static_cast<uint32_t>(dest->Width), static_cast<uint32_t>(dest->Height), 1 },
		};

		VkCopyBufferToImageInfo2 copyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
			.srcBuffer = src->Allocation.Resource,
			.dstImage = dest->Allocation.Resource,
			.dstImageLayout = dest->Layout,
			.regionCount = 1,
			.pRegions = &bufferImageCopy,
		};

		vkCmdCopyBufferToImage2(m_Impl->Resource, &copyInfo);
	}

	void CommandList::SetPushConstants(GraphicsPipeline pipeline, const void* data, uint32_t size) const
	{
		vkCmdPushConstants(m_Impl->Resource, pipeline->Layout, VK_SHADER_STAGE_ALL, 0, size, data);
	}

	void CommandList::Draw(uint32_t vertexCount) const
	{
		vkCmdDraw(m_Impl->Resource, vertexCount, 1, 0, 0);
	}

	void CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceIndex) const
	{
		vkCmdDrawIndexed(m_Impl->Resource, indexCount, 1, 0, 0, instanceIndex);
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

	void CommandPool::Destroy()
	{
		vkDestroyCommandPool(m_Impl->Context->Device, m_Impl->Resource, nullptr);
		delete m_Impl;
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
