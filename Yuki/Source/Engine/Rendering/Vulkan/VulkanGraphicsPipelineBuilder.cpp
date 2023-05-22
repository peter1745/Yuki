#include "VulkanGraphicsPipelineBuilder.hpp"
#include "VulkanRenderContext.hpp"
#include "VulkanGraphicsPipeline.hpp"

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
		}

		YUKI_VERIFY(false);
		return VK_FORMAT_UNDEFINED;
	}

	static constexpr VkFormat ImageFormatToVkFormat(ImageFormat InFormat)
	{
		switch (InFormat)
		{
		case ImageFormat::RGBA8UNorm: return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::BGRA8UNorm: return VK_FORMAT_B8G8R8A8_UNORM;
		case ImageFormat::D24UNormS8UInt: return VK_FORMAT_D24_UNORM_S8_UINT;
		}

		YUKI_VERIFY(false);
		return VK_FORMAT_UNDEFINED;
	}

	VulkanGraphicsPipelineBuilder::VulkanGraphicsPipelineBuilder(RenderContext* InContext)
	    : m_ShaderManager(InContext->GetShaderManager())
	{
		m_Device = ((VulkanDevice*)InContext->GetDevice())->GetLogicalDevice();
	}

	GraphicsPipelineBuilder* VulkanGraphicsPipelineBuilder::WithShader(ResourceHandle<Shader> InShaderHandle)
	{
		Shader* shader = m_ShaderManager->GetShader(InShaderHandle);

		m_ShaderStageInfos.reserve(shader->ModuleHandles.size());

		for (const auto& [moduleType, moduleHandle] : shader->ModuleHandles)
		{
			auto& stageCreateInfo = m_ShaderStageInfos.emplace_back();
			stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageCreateInfo.stage = ShaderModuleTypeToVkStageFlagBits(moduleType);
			stageCreateInfo.module = reinterpret_cast<VkShaderModule>(moduleHandle);
			stageCreateInfo.pName = "main";
		}

		return this;
	}

	GraphicsPipelineBuilder* VulkanGraphicsPipelineBuilder::AddVertexInput(uint32_t InLocation, ShaderDataType InDataType)
	{
		auto& vertexInputAttribute = m_VertexInputAttributes.emplace_back();
		vertexInputAttribute.location = InLocation;
		vertexInputAttribute.binding = 0;
		vertexInputAttribute.format = ShaderDataTypeToVkFormat(InDataType);
		vertexInputAttribute.offset = m_VertexInputAttributesOffset;

		m_VertexInputAttributesOffset += ShaderDataTypeSize(InDataType);

		return this;
	}

	GraphicsPipelineBuilder* VulkanGraphicsPipelineBuilder::ColorAttachment(ImageFormat InFormat)
	{
		YUKI_VERIFY(InFormat != ImageFormat::D24UNormS8UInt);

		m_ColorAttachmentFormats.emplace_back(ImageFormatToVkFormat(InFormat));

		auto& blendStateInfo = m_ColorAttachmentBlendStates.emplace_back();
		blendStateInfo.blendEnable = VK_TRUE;
		blendStateInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendStateInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendStateInfo.colorBlendOp = VK_BLEND_OP_ADD;
		blendStateInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendStateInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendStateInfo.alphaBlendOp = VK_BLEND_OP_ADD;
		blendStateInfo.colorWriteMask = 0xF;

		return this;
	}

	GraphicsPipelineBuilder* VulkanGraphicsPipelineBuilder::DepthAttachment()
	{
		m_HasDepthAttachment = true;
		return this;
	}

	Unique<GraphicsPipeline> VulkanGraphicsPipelineBuilder::Build()
	{
		VkPipelineRenderingCreateInfo renderingCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = uint32_t(m_ColorAttachmentFormats.size()),
			.pColorAttachmentFormats = m_ColorAttachmentFormats.data(),
			.depthAttachmentFormat = m_HasDepthAttachment ? VK_FORMAT_D24_UNORM_S8_UINT : VK_FORMAT_UNDEFINED,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
		};

		VkPipelineLayoutCreateInfo layoutCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 0,
			.pSetLayouts = nullptr,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};

		VkPipelineLayout pipelineLayout;
		YUKI_VERIFY(vkCreatePipelineLayout(m_Device, &layoutCreateInfo, nullptr, &pipelineLayout) == VK_SUCCESS);

		VkVertexInputBindingDescription vertexInputBindingDesc = {
			.binding = 0,
			.stride = m_VertexInputAttributesOffset,
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};

		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &vertexInputBindingDesc,
			.vertexAttributeDescriptionCount = uint32_t(m_VertexInputAttributes.size()),
			.pVertexAttributeDescriptions = m_VertexInputAttributes.data()
		};

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
		};

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1
		};

		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
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

		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE
		};

		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = m_HasDepthAttachment ? VK_TRUE : VK_FALSE,
			.depthWriteEnable = m_HasDepthAttachment ? VK_TRUE : VK_FALSE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
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

		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.attachmentCount = uint32_t(m_ColorAttachmentBlendStates.size()),
			.pAttachments = m_ColorAttachmentBlendStates.data()
		};

		constexpr auto dynamicStates = std::array { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_POLYGON_MODE_EXT };

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = uint32_t(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
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

		Unique<VulkanGraphicsPipeline> result = Unique<VulkanGraphicsPipeline>::Create();
		YUKI_VERIFY(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &result->Pipeline) == VK_SUCCESS);
		result->Layout = pipelineLayout;

		return result;
	}

}
