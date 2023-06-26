#include "Rendering/PipelineBuilder.hpp"
#include "Rendering/RenderContext.hpp"
#include "Rendering/RenderResources.hpp"

namespace Yuki {

	PipelineBuilder::PipelineBuilder(RenderContext* InContext)
		: m_Context(InContext)
	{
		
	}

	PipelineBuilder& PipelineBuilder::WithShader(ShaderHandle InShader)
	{
		m_PipelineInfo.PipelineShader = InShader;
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

	PipelineBuilder& PipelineBuilder::AddDescriptorSetLayout(DescriptorSetLayoutHandle InLayout)
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
	
	PipelineBuilder& PipelineBuilder::SetPolygonMode(PolygonModeType InPolygonMode)
	{
		m_PipelineInfo.PolygonMode = InPolygonMode;
		return *this;
	}

	Pipeline PipelineBuilder::Build()
	{
		return { m_Context->CreatePipeline(m_PipelineInfo), m_Context };
	}

}
