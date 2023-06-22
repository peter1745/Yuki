#include "VulkanGraphicsPipelineBuilder.hpp"
#include "VulkanRenderContext.hpp"
#include "VulkanGraphicsPipeline.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	static constexpr VkShaderStageFlagBits ShaderModuleTypeToVkStageFlagBits(ShaderModuleType InType)
	{
		switch (InType)
		{
		case ShaderModuleType::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderModuleType::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
		default:
			break;
		}

		YUKI_VERIFY(false);
		return static_cast<VkShaderStageFlagBits>(-1);
	}

	static constexpr VkFormat ShaderDataTypeToVkFormat(ShaderDataType InDataType)
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

	VulkanGraphicsPipelineBuilder::VulkanGraphicsPipelineBuilder(VulkanRenderContext* InContext)
		: m_Context(InContext)
	{
	}

	GraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::Start()
	{
		m_PipelineShader = nullptr;
		m_ShaderStageInfos.clear();
		m_VertexInputAttributes.clear();
		m_VertexInputAttributesOffset = 0;
		m_PushConstants.clear();
		m_ColorAttachmentFormats.clear();
		m_ColorAttachmentBlendStates.clear();
		m_HasDepthAttachment = false;

		return *this;
	}

	GraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::WithShader(Shader* InShader)
	{
		m_PipelineShader = static_cast<VulkanShader*>(InShader);

		m_ShaderStageInfos.reserve(m_PipelineShader->m_ModuleHandles.size());

		for (const auto& [moduleType, moduleHandle] : m_PipelineShader->m_ModuleHandles)
		{
			auto& stageCreateInfo = m_ShaderStageInfos.emplace_back();
			stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageCreateInfo.stage = ShaderModuleTypeToVkStageFlagBits(moduleType);
			stageCreateInfo.module = reinterpret_cast<VkShaderModule>(moduleHandle);
			stageCreateInfo.pName = "main";
		}

		return *this;
	}

	GraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::AddVertexInput(uint32_t InLocation, ShaderDataType InDataType)
	{
		auto& vertexInputAttribute = m_VertexInputAttributes.emplace_back();
		vertexInputAttribute.location = InLocation;
		vertexInputAttribute.binding = 0;
		vertexInputAttribute.format = ShaderDataTypeToVkFormat(InDataType);
		vertexInputAttribute.offset = m_VertexInputAttributesOffset;

		m_VertexInputAttributesOffset += ShaderDataTypeSize(InDataType);

		return *this;
	}

	GraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::PushConstant(uint32_t InOffset, uint32_t InSize)
	{
		VkPushConstantRange range =
		{
			.stageFlags = VK_SHADER_STAGE_ALL,
			.offset = InOffset,
			.size = InSize,
		};
		m_PushConstants.emplace_back(std::move(range));
		
		return *this;
	}

	GraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::AddDescriptorSetLayout(DescriptorSetLayout* InLayout)
	{
		m_DescriptorSetLayouts.emplace_back(static_cast<VulkanDescriptorSetLayout*>(InLayout));
		return *this;
	}

	GraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::ColorAttachment(ImageFormat InFormat)
	{
		YUKI_VERIFY(InFormat != ImageFormat::Depth32SFloat);

		m_ColorAttachmentFormats.emplace_back(VulkanHelper::ImageFormatToVkFormat(InFormat));

		auto& blendStateInfo = m_ColorAttachmentBlendStates.emplace_back();
		blendStateInfo.blendEnable = VK_TRUE;
		blendStateInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendStateInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendStateInfo.colorBlendOp = VK_BLEND_OP_ADD;
		blendStateInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendStateInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendStateInfo.alphaBlendOp = VK_BLEND_OP_ADD;
		blendStateInfo.colorWriteMask = 0xF;

		return *this;
	}

	GraphicsPipelineBuilder& VulkanGraphicsPipelineBuilder::DepthAttachment()
	{
		m_HasDepthAttachment = true;
		return *this;
	}

	Unique<GraphicsPipeline> VulkanGraphicsPipelineBuilder::Build()
	{
		VkPipelineRenderingCreateInfo renderingCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = uint32_t(m_ColorAttachmentFormats.size()),
			.pColorAttachmentFormats = m_ColorAttachmentFormats.data(),
			.depthAttachmentFormat = m_HasDepthAttachment ? VK_FORMAT_D32_SFLOAT : VK_FORMAT_UNDEFINED,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
		};

		List<VkDescriptorSetLayout> descriptorSetLayouts;
		descriptorSetLayouts.reserve(m_DescriptorSetLayouts.size());
		for (auto* descriptorSetLayout : m_DescriptorSetLayouts)
			descriptorSetLayouts.emplace_back(descriptorSetLayout->Handle);

		VkPipelineLayoutCreateInfo layoutCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = uint32_t(descriptorSetLayouts.size()),
			.pSetLayouts = descriptorSetLayouts.data(),
			.pushConstantRangeCount = uint32_t(m_PushConstants.size()),
			.pPushConstantRanges = m_PushConstants.data(),
		};

		VkPipelineLayout pipelineLayout;
		YUKI_VERIFY(vkCreatePipelineLayout(m_Context->GetDevice(), &layoutCreateInfo, nullptr, &pipelineLayout) == VK_SUCCESS);

		VkVertexInputBindingDescription vertexInputBindingDesc =
		{
			.binding = 0,
			.stride = m_VertexInputAttributesOffset,
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};

		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &vertexInputBindingDesc,
			.vertexAttributeDescriptionCount = uint32_t(m_VertexInputAttributes.size()),
			.pVertexAttributeDescriptions = m_VertexInputAttributes.data()
		};

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
			.polygonMode = VK_POLYGON_MODE_FILL,
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
			.depthTestEnable = m_HasDepthAttachment ? VK_TRUE : VK_FALSE,
			.depthWriteEnable = m_HasDepthAttachment ? VK_TRUE : VK_FALSE,
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
			.attachmentCount = uint32_t(m_ColorAttachmentBlendStates.size()),
			.pAttachments = m_ColorAttachmentBlendStates.data()
		};

		constexpr auto dynamicStates = std::array { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = uint32_t(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &renderingCreateInfo,
			.stageCount = uint32_t(m_ShaderStageInfos.size()),
			.pStages = m_ShaderStageInfos.data(),
			.pVertexInputState = &vertexInputStateCreateInfo,
			.pInputAssemblyState = &inputAssemblyStateCreateInfo,
			.pViewportState = &viewportStateCreateInfo,
			.pRasterizationState = &rasterizationStateCreateInfo,
			.pMultisampleState = &multisampleStateCreateInfo,
			.pDepthStencilState = &depthStencilCreateInfo,
			.pColorBlendState = &colorBlendStateCreateInfo,
			.pDynamicState = &dynamicStateCreateInfo,
			.layout = pipelineLayout,
		};

		auto* result = new VulkanGraphicsPipeline();
		result->m_Context = m_Context;
		result->m_Shader = m_PipelineShader;
		result->m_Layout = pipelineLayout;

		YUKI_VERIFY(vkCreateGraphicsPipelines(m_Context->GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &result->m_Pipeline) == VK_SUCCESS);

		return result;
	}

}
