#include "VulkanRHI.hpp"

#include <Aura/Stack.hpp>

namespace Yuki {

	GraphicsPipeline GraphicsPipeline::Create(RHIContext context, const GraphicsPipelineConfig& config, DescriptorHeap heap)
	{
		AuraStackPoint();

		auto* impl = new Impl();
		impl->Context = context;

		auto shaderModules = Aura::StackAlloc<VkPipelineShaderStageCreateInfo>(config.Shaders.size());
		for (uint32_t i = 0; i < config.Shaders.size(); i++)
		{
			const auto& shaderConfig = config.Shaders[i];

			shaderModules[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderModules[i].stage = ShaderStageToVkShaderStage(shaderConfig.Stage);
			shaderModules[i].module = context->Compiler->CompileShader(context, shaderConfig.FilePath, shaderConfig.Stage);
			shaderModules[i].pName = "main";
		}

		auto colorAttachmentFormats = Aura::StackAlloc<VkFormat>(config.ColorAttachmentFormats.size());
		auto colorAttachmentBlendStates = Aura::StackAlloc<VkPipelineColorBlendAttachmentState>(config.ColorAttachmentFormats.size());
		for (uint32_t i = 0; i < config.ColorAttachmentFormats.size(); i++)
		{
			colorAttachmentFormats[i] = ImageFormatToVkFormat(config.ColorAttachmentFormats[i]);

			colorAttachmentBlendStates[i].blendEnable = VK_FALSE;
			colorAttachmentBlendStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorAttachmentBlendStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorAttachmentBlendStates[i].colorBlendOp = VK_BLEND_OP_ADD;
			colorAttachmentBlendStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorAttachmentBlendStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorAttachmentBlendStates[i].alphaBlendOp = VK_BLEND_OP_ADD;
			colorAttachmentBlendStates[i].colorWriteMask = 0xF;
		}

		VkPipelineRenderingCreateInfo renderingInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = colorAttachmentFormats.Count(),
			.pColorAttachmentFormats = colorAttachmentFormats.Data(),
		};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};

		VkPipelineViewportStateCreateInfo viewportInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, };

		VkPipelineRasterizationStateCreateInfo rasterizationInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = VK_FALSE,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f,
		};

		VkPipelineMultisampleStateCreateInfo multisampleInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE
		};

		VkPipelineDepthStencilStateCreateInfo depthStencilInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
		};

		VkPipelineColorBlendStateCreateInfo colorBlendInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.attachmentCount = colorAttachmentBlendStates.Count(),
			.pAttachments = colorAttachmentBlendStates.Data()
		};

		constexpr auto dynamicStates = std::array{
			VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
			VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,
		};

		VkPipelineDynamicStateCreateInfo dynamicInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data(),
		};

		VkPushConstantRange pushConstantRange =
		{
			.stageFlags = VK_SHADER_STAGE_ALL,
			.offset = 0,
			.size = config.PushConstantSize
		};

		VkPipelineLayoutCreateInfo layoutInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = heap ? 1u : 0u,
			.pSetLayouts = heap ? &heap->Layout : nullptr,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &pushConstantRange,
		};

		Vulkan::CheckResult(vkCreatePipelineLayout(context->Device, &layoutInfo, nullptr, &impl->Layout));

		VkGraphicsPipelineCreateInfo pipelineInfo =
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &renderingInfo,
			.stageCount = shaderModules.Count(),
			.pStages = shaderModules.Data(),
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssemblyInfo,
			.pViewportState = &viewportInfo,
			.pRasterizationState = &rasterizationInfo,
			.pMultisampleState = &multisampleInfo,
			.pDepthStencilState = &depthStencilInfo,
			.pColorBlendState = &colorBlendInfo,
			.pDynamicState = &dynamicInfo,
			.layout = impl->Layout,
		};

		Vulkan::CheckResult(vkCreateGraphicsPipelines(context->Device, nullptr, 1, &pipelineInfo, nullptr, &impl->Resource));

		return { impl };
	}

	void GraphicsPipeline::Destroy()
	{
		vkDestroyPipeline(m_Impl->Context->Device, m_Impl->Resource, nullptr);
		vkDestroyPipelineLayout(m_Impl->Context->Device, m_Impl->Layout, nullptr);
		delete m_Impl;
	}

}
