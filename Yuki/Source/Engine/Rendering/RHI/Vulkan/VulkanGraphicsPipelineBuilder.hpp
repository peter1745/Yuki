#pragma once

#include "Rendering/RHI/GraphicsPipelineBuilder.hpp"

#include "Vulkan.hpp"

namespace Yuki {

	class VulkanGraphicsPipelineBuilder : public GraphicsPipelineBuilder
	{
	public:
		VulkanGraphicsPipelineBuilder(RenderContext* InContext);

		GraphicsPipelineBuilder* WithShader(ResourceHandle<Shader> InShaderHandle) override;
		GraphicsPipelineBuilder* AddVertexInput(uint32_t InLocation, ShaderDataType InDataType) override;
		GraphicsPipelineBuilder* PushConstant(uint32_t InOffset, uint32_t InSize) override;
		GraphicsPipelineBuilder* ColorAttachment(ImageFormat InFormat) override;
		GraphicsPipelineBuilder* DepthAttachment() override;
		Unique<GraphicsPipeline> Build() override;

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		ShaderManager* m_ShaderManager = nullptr;
		Shader* m_PipelineShader = nullptr;

		List<VkPipelineShaderStageCreateInfo> m_ShaderStageInfos;
		List<VkVertexInputAttributeDescription> m_VertexInputAttributes;
		uint32_t m_VertexInputAttributesOffset = 0;

		List<VkPushConstantRange> m_PushConstants;

		List<VkFormat> m_ColorAttachmentFormats;
		List<VkPipelineColorBlendAttachmentState> m_ColorAttachmentBlendStates;

		bool m_HasDepthAttachment = false;
	};

}
