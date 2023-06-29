#include "ImGuiVulkanRenderContext.hpp"

#include "Rendering/DescriptorSetBuilder.hpp"

#include "../Rendering/Vulkan/VulkanRenderContext.hpp"

#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES
#include <imgui/backends/imgui_impl_vulkan.h>

namespace Yuki {

	ImGuiVulkanRenderContext::ImGuiVulkanRenderContext(SwapchainHandle InSwapchain, RenderContext* InContext)
	{
		auto queue = Queue{InContext->GetGraphicsQueue(), InContext};
		auto* context = static_cast<VulkanRenderContext*>(InContext);
		auto& swapchain = context->m_Swapchains.Get(InSwapchain);

		ImGui_ImplVulkan_LoadFunctions([](const char *function_name, void *vulkan_instance)
		{
    		return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance*>(vulkan_instance)), function_name);
		}, &context->m_Instance);

		auto descriptorPoolSizes = std::array
		{
			DescriptorCount{ DescriptorType::Sampler, 1000 },
			DescriptorCount{ DescriptorType::CombinedImageSampler, 1000 },
			DescriptorCount{ DescriptorType::SampledImage, 1000 },
			DescriptorCount{ DescriptorType::StorageImage, 1000 },
			DescriptorCount{ DescriptorType::UniformTexelBuffer, 1000 },
			DescriptorCount{ DescriptorType::StorageTexelBuffer, 1000 },
			DescriptorCount{ DescriptorType::UniformBuffer, 1000 },
			DescriptorCount{ DescriptorType::StorageBuffer, 1000 },
			DescriptorCount{ DescriptorType::UniformBufferDynamic, 1000 },
			DescriptorCount{ DescriptorType::StorageBufferDynamic, 1000 },
			DescriptorCount{ DescriptorType::InputAttachment, 1000 },
		};

		m_DescriptorPool = DescriptorPool(InContext, descriptorPoolSizes);

		VkAttachmentDescription attachmentDesc =
		{
			.flags = 0,
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
		};

		VkAttachmentReference attachmentReference =
		{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription subpassDesc =
		{
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &attachmentReference,
		};

		VkSubpassDependency subpassDependency =
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
		};

		VkRenderPassCreateInfo renderPassInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.attachmentCount = 1,
			.pAttachments = &attachmentDesc,
			.subpassCount = 1,
			.pSubpasses = &subpassDesc,
			.dependencyCount = 1,
			.pDependencies = &subpassDependency,
		};
		vkCreateRenderPass(context->m_LogicalDevice, &renderPassInfo, nullptr, &m_RenderPass);

		ImGui_ImplVulkan_InitInfo vulkanInitInfo =
		{
			.Instance = context->m_Instance,
			.PhysicalDevice = context->m_PhysicalDevice,
			.Device = context->m_LogicalDevice,
			.QueueFamily = context->m_Queues.Get(queue).FamilyIndex,
			.Queue = context->m_Queues.Get(queue).Queue,
			.PipelineCache = VK_NULL_HANDLE,
			.DescriptorPool = context->m_DescriptorPools.Get(m_DescriptorPool).Handle,
			.Subpass = 0,
			.MinImageCount = uint32_t(swapchain.Images.size()),
			.ImageCount = uint32_t(swapchain.Images.size()),
			.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
			.Allocator = nullptr
		};
		ImGui_ImplVulkan_Init(&vulkanInitInfo, m_RenderPass);

		{
			CommandPool pool{InContext, queue};
			auto commandList = pool.CreateCommandList();
			commandList.Begin();
			ImGui_ImplVulkan_CreateFontsTexture(context->m_CommandLists.Get(commandList).CommandBuffer);
			commandList.End();
			context->QueueSubmitCommandLists(queue, { commandList }, {}, {});
			context->QueueWaitIdle(queue);
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		m_Swapchain = &swapchain;
		m_Context = InContext;
	}

	void ImGuiVulkanRenderContext::NewFrame(CommandListHandle InCommandList) const
	{
		ImGui_ImplVulkan_NewFrame();
	}

	void ImGuiVulkanRenderContext::EndFrame(CommandListHandle InCommandList)
	{
		auto* context = static_cast<VulkanRenderContext*>(m_Context);
		CommandList commandList{InCommandList, m_Context};

		const auto& currentImage = context->m_Images.Get(m_Swapchain->Images[m_Swapchain->CurrentImage]);

		if (m_LastWidth != currentImage.Width || m_LastHeight != currentImage.Height)
		{
			m_LastWidth = currentImage.Width;
			m_LastHeight = currentImage.Height;

			vkDestroyFramebuffer(context->m_LogicalDevice, m_Framebuffer, nullptr);

			VkFramebufferAttachmentImageInfo attachmentImageInfo =
			{
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO,
				.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				.width = m_LastWidth,
				.height = m_LastHeight,
				.layerCount = 1,
				.viewFormatCount = 1,
				.pViewFormats = &currentImage.Format,
			};

			VkFramebufferAttachmentsCreateInfo attachmentInfo =
			{
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO,
				.attachmentImageInfoCount = 1,
				.pAttachmentImageInfos = &attachmentImageInfo,
			};

			VkFramebufferCreateInfo framebufferInfo =
			{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = &attachmentInfo,
                .flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT,
                .renderPass = m_RenderPass,
                .attachmentCount = 1,
                .width = m_LastWidth,
                .height = m_LastHeight,
                .layers = 1,
            };
            vkCreateFramebuffer(context->m_LogicalDevice, &framebufferInfo, nullptr, &m_Framebuffer);
		}

		commandList.TransitionImage(m_Swapchain->Images[m_Swapchain->CurrentImage], ImageLayout::Attachment);

		VkRenderPassAttachmentBeginInfo renderPassAttachmentBegin =
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO,
			.attachmentCount = 1,
			.pAttachments = &context->m_ImageViews.Get(m_Swapchain->ImageViews[m_Swapchain->CurrentImage]).ImageView,
		};

		VkRenderPassBeginInfo renderPassBegin =
		{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = &renderPassAttachmentBegin,
            .renderPass = m_RenderPass,
            .framebuffer = m_Framebuffer,
            .renderArea = { {}, { m_LastWidth, m_LastHeight } },
        };

		vkCmdBeginRenderPass(context->m_CommandLists.Get(InCommandList).CommandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), context->m_CommandLists.Get(commandList).CommandBuffer);
		vkCmdEndRenderPass(context->m_CommandLists.Get(commandList).CommandBuffer);
	}

}
