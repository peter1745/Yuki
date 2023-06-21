#pragma once

#include "Rendering/RHI/GraphicsPipelineBuilder.hpp"

#include "VulkanSetLayoutBuilder.hpp"
#include "VulkanShader.hpp"

namespace Yuki {

	class VulkanRenderContext;

	class VulkanGraphicsPipelineBuilder : public GraphicsPipelineBuilder
	{
	public:
		VulkanGraphicsPipelineBuilder(VulkanRenderContext* InContext);

		GraphicsPipelineBuilder& Start() override;
		GraphicsPipelineBuilder& WithShader(Shader* InShader) override;
		GraphicsPipelineBuilder& AddVertexInput(uint32_t InLocation, ShaderDataType InDataType) override;
		GraphicsPipelineBuilder& PushConstant(uint32_t InOffset, uint32_t InSize) override;
		GraphicsPipelineBuilder& AddDescriptorSetLayout(DescriptorSetLayout* InLayout) override;
		GraphicsPipelineBuilder& ColorAttachment(ImageFormat InFormat) override;
		GraphicsPipelineBuilder& DepthAttachment() override;
		Unique<GraphicsPipeline> Build() override;

	private:
		VulkanRenderContext* m_Context = nullptr;
		VulkanShader* m_PipelineShader = nullptr;

		List<VkPipelineShaderStageCreateInfo> m_ShaderStageInfos;
		List<VkVertexInputAttributeDescription> m_VertexInputAttributes;
		uint32_t m_VertexInputAttributesOffset = 0;

		List<VkPushConstantRange> m_PushConstants;

		List<VulkanDescriptorSetLayout*> m_DescriptorSetLayouts;

		List<VkFormat> m_ColorAttachmentFormats;
		List<VkPipelineColorBlendAttachmentState> m_ColorAttachmentBlendStates;

		bool m_HasDepthAttachment = false;
	};

}
