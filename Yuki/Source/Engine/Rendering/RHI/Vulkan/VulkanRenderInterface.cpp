#include "VulkanRenderInterface.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanImage2D.hpp"
#include "VulkanGraphicsPipeline.hpp"

namespace Yuki {

	void VulkanRenderInterface::BeginRendering(CommandBuffer* InCmdBuffer, RenderTarget* InRenderTarget)
	{
		VulkanRenderTarget* renderTarget = (VulkanRenderTarget*)InRenderTarget;

		const auto& renderTargetInfo = renderTarget->GetInfo();

		// Transition Images
		{
			VulkanImageTransition imageTransition =
			{
				.DstPipelineStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
				.DstAccessFlags = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
				.DstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};

			for (const auto& colorAttachmentInfo : renderTarget->GetColorAttachmentInfos())
				((VulkanImage2D*)colorAttachmentInfo.Image)->Transition(InCmdBuffer->As<VkCommandBuffer>(), imageTransition);

			const auto& depthAttachmentInfo = renderTarget->GetDepthAttachmentInfo();
			if (depthAttachmentInfo.IsValid())
			{
				VulkanImageTransition depthImageTransition =
				{
					.DstPipelineStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
					.DstAccessFlags = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
					.DstImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
				};

				((VulkanImage2D*)depthAttachmentInfo.Image)->Transition(InCmdBuffer->As<VkCommandBuffer>(), depthImageTransition);
			}
		}

		const auto& colorRenderingAttachments = renderTarget->GetColorRenderingAttachments();
		const auto& depthRenderingAttachment = renderTarget->GetDepthRenderingAttachment();

		VkRenderingInfo renderingInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea = {
			    .offset = { 0, 0 },
			    .extent = { renderTargetInfo.Width, renderTargetInfo.Height },
			},
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = uint32_t(colorRenderingAttachments.size()),
			.pColorAttachments = colorRenderingAttachments.data(),
			.pDepthAttachment = &depthRenderingAttachment
		};

		vkCmdBeginRendering(InCmdBuffer->As<VkCommandBuffer>(), &renderingInfo);
	}

	void VulkanRenderInterface::EndRendering(CommandBuffer* InCmdBuffer)
	{
		vkCmdEndRendering(InCmdBuffer->As<VkCommandBuffer>());
	}

}
