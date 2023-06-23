#include "Rendering/PipelineBuilder.hpp"
#include "Rendering/RenderContext.hpp"

namespace Yuki {

	PipelineBuilder::PipelineBuilder(RenderContext* InContext)
		: m_Context(InContext)
	{
		
	}

	PipelineBuilder& PipelineBuilder::WithShader(Shader InShader)
	{
		m_PipelineInfo.PipelineShader = InShader;
		return *this;
	}
	
	PipelineBuilder& PipelineBuilder::AddVertexInput(ShaderDataType InDataType)
	{
		m_PipelineInfo.VertexInputs.emplace_back(InDataType);
		m_PipelineInfo.VertexStride += ShaderDataTypeSize(InDataType);
		return *this;
	}
	
	PipelineBuilder& PipelineBuilder::PushConstant(uint32_t InSize)
	{
		auto& pushConstantInfo = m_PipelineInfo.PushConstants.emplace_back();
		pushConstantInfo.Offset = m_PushConstantOffset;
		pushConstantInfo.Size = InSize;

		m_PushConstantOffset += InSize;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::AddDescriptorSetLayout(DescriptorSetLayout InLayout)
	{
		m_PipelineInfo.DescriptorSetLayouts.emplace_back(InLayout);
		return *this;
	}
	
	PipelineBuilder& PipelineBuilder::ColorAttachment(ImageFormat InFormat)
	{
		auto& colorAttachmentInfo = m_PipelineInfo.ColorAttachments.emplace_back();
		colorAttachmentInfo.Format = InFormat;
		return *this;
	}
	
	PipelineBuilder& PipelineBuilder::DepthAttachment()
	{
		m_PipelineInfo.HasDepthAttachment = true;
		return *this;
	}
	
	Pipeline PipelineBuilder::Build()
	{
		return m_Context->CreatePipeline(m_PipelineInfo);
	}

}
