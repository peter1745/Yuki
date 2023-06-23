#include "VulkanPipeline.hpp"
#include "VulkanRenderContext.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	static VkFormat ShaderDataTypeToVkFormat(ShaderDataType InDataType)
	{
		switch (InDataType)
		{
		case ShaderDataType::Float: return VK_FORMAT_R32_SFLOAT;
		case ShaderDataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
		case ShaderDataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
		case ShaderDataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case ShaderDataType::Int: return VK_FORMAT_R32_SINT;
		case ShaderDataType::Int2: return VK_FORMAT_R32G32_SINT;
		case ShaderDataType::Int3: return VK_FORMAT_R32G32B32_SINT;
		case ShaderDataType::Int4: return VK_FORMAT_R32G32B32A32_SINT;
		case ShaderDataType::UInt: return VK_FORMAT_R32_UINT;
		case ShaderDataType::UInt2: return VK_FORMAT_R32G32_UINT;
		case ShaderDataType::UInt3: return VK_FORMAT_R32G32B32_UINT;
		case ShaderDataType::UInt4: return VK_FORMAT_R32G32B32A32_UINT;
		}

		YUKI_VERIFY(false);
		return VK_FORMAT_UNDEFINED;
	}

	static VkShaderStageFlagBits ShaderModuleTypeToVkShaderStageFlagBits(ShaderModuleType InType)
	{
		switch (InType)
		{
		case ShaderModuleType::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderModuleType::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		return (VkShaderStageFlagBits)-1;
	}

	Pipeline VulkanRenderContext::CreatePipeline(const PipelineInfo& InPipelineInfo)
	{
		auto[handle, pipeline] = m_Pipelines.Acquire();

		DynamicArray<VkFormat> colorAttachmentFormats;
		DynamicArray<VkPipelineColorBlendAttachmentState> colorAttachmentBlendStates;
		for (const auto& colorAttachmentInfo : InPipelineInfo.ColorAttachments)
		{
			colorAttachmentFormats.emplace_back(VulkanHelper::ImageFormatToVkFormat(colorAttachmentInfo.Format));

			auto& blendStateInfo = colorAttachmentBlendStates.emplace_back();
			blendStateInfo.blendEnable = VK_TRUE;
			blendStateInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendStateInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendStateInfo.colorBlendOp = VK_BLEND_OP_ADD;
			blendStateInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendStateInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendStateInfo.alphaBlendOp = VK_BLEND_OP_ADD;
			blendStateInfo.colorWriteMask = 0xF;
		}

		VkPipelineRenderingCreateInfo renderingCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = uint32_t(colorAttachmentFormats.size()),
			.pColorAttachmentFormats = colorAttachmentFormats.data(),
			.depthAttachmentFormat = InPipelineInfo.HasDepthAttachment ? VK_FORMAT_D32_SFLOAT : VK_FORMAT_UNDEFINED,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
		};

		DynamicArray<VkDescriptorSetLayout> descriptorSetLayouts;
		descriptorSetLayouts.reserve(InPipelineInfo.DescriptorSetLayouts.size());
		for (auto layoutHandle : InPipelineInfo.DescriptorSetLayouts)
			descriptorSetLayouts.emplace_back(m_DescriptorSetLayouts.Get(layoutHandle).Handle);

		DynamicArray<VkPushConstantRange> pushConstants;
		for (const auto& pushConstantInfo : InPipelineInfo.PushConstants)
		{
			auto& pushConstantRange = pushConstants.emplace_back();
			pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
			pushConstantRange.offset = pushConstantInfo.Offset;
			pushConstantRange.size = pushConstantInfo.Size;
		}

		VkPipelineLayoutCreateInfo layoutCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = uint32_t(descriptorSetLayouts.size()),
			.pSetLayouts = descriptorSetLayouts.data(),
			.pushConstantRangeCount = uint32_t(pushConstants.size()),
			.pPushConstantRanges = pushConstants.data(),
		};
		YUKI_VERIFY(vkCreatePipelineLayout(m_LogicalDevice, &layoutCreateInfo, nullptr, &pipeline.Layout) == VK_SUCCESS);

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
		};

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1
		};

		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = InPipelineInfo.PolygonMode == PolygonModeType::Fill ? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE,
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = VK_FALSE,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f,
		};

		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE
		};

		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = InPipelineInfo.HasDepthAttachment ? VK_TRUE : VK_FALSE,
			.depthWriteEnable = InPipelineInfo.HasDepthAttachment ? VK_TRUE : VK_FALSE,
			.depthCompareOp = VK_COMPARE_OP_GREATER,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = {
				.failOp = VK_STENCIL_OP_KEEP,
			    .passOp = VK_STENCIL_OP_KEEP,
				.depthFailOp = VK_STENCIL_OP_KEEP,
				.compareOp = VK_COMPARE_OP_ALWAYS,
				.compareMask = 0xFF,
			    .writeMask = 0xFF,
				.reference = 0,
			},
			.back = {
			    .failOp = VK_STENCIL_OP_KEEP,
			    .passOp = VK_STENCIL_OP_KEEP,
			    .depthFailOp = VK_STENCIL_OP_KEEP,
			    .compareOp = VK_COMPARE_OP_ALWAYS,
			    .compareMask = 0xFF,
			    .writeMask = 0xFF,
			    .reference = 0,
			},
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 0.0f,
		};

		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.attachmentCount = uint32_t(colorAttachmentBlendStates.size()),
			.pAttachments = colorAttachmentBlendStates.data()
		};

		constexpr auto dynamicStates = std::array { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = uint32_t(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		auto& shader = m_Shaders.Get(InPipelineInfo.PipelineShader);
		DynamicArray<VkPipelineShaderStageCreateInfo> shaderStages;
		for (const auto& [moduleType, moduleHandle] : shader.Modules)
		{
			auto& stageCreateInfo = shaderStages.emplace_back();
			stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageCreateInfo.stage = ShaderModuleTypeToVkShaderStageFlagBits(moduleType);
			stageCreateInfo.module = moduleHandle;
			stageCreateInfo.pName = "main";
		}

		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, };
		VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &renderingCreateInfo,
			.stageCount = uint32_t(shaderStages.size()),
			.pStages = shaderStages.data(),
			.pVertexInputState = &vertexInputStateCreateInfo,
			.pInputAssemblyState = &inputAssemblyStateCreateInfo,
			.pViewportState = &viewportStateCreateInfo,
			.pRasterizationState = &rasterizationStateCreateInfo,
			.pMultisampleState = &multisampleStateCreateInfo,
			.pDepthStencilState = &depthStencilCreateInfo,
			.pColorBlendState = &colorBlendStateCreateInfo,
			.pDynamicState = &dynamicStateCreateInfo,
			.layout = pipeline.Layout,
		};

		YUKI_VERIFY(vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline.Pipeline) == VK_SUCCESS);
		return handle;
	}

	void VulkanRenderContext::Destroy(Pipeline InPipeline)
	{
		auto& pipeline = m_Pipelines.Get(InPipeline);
		vkDestroyPipeline(m_LogicalDevice, pipeline.Pipeline, nullptr);
		vkDestroyPipelineLayout(m_LogicalDevice, pipeline.Layout, nullptr);
		m_Pipelines.Return(InPipeline);
	}

}
