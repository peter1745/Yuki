#pragma once

#include "Yuki/Core/Core.hpp"

#include "RHI.hpp"

namespace Yuki {

	class RenderContext;
	struct Pipeline;

	struct PipelineInfo
	{
		ShaderHandle PipelineShader = {};
		
		struct PushConstantInfo { uint32_t Offset; uint32_t Size; };
		DynamicArray<PushConstantInfo> PushConstants;

		DynamicArray<DescriptorSetLayoutHandle> DescriptorSetLayouts;

		struct ColorAttachmentInfo
		{
			ImageFormat Format;
		};
		DynamicArray<ColorAttachmentInfo> ColorAttachments;

		bool HasDepthAttachment = false;

		PolygonModeType PolygonMode = PolygonModeType::Fill; 
	};

	class PipelineBuilder
	{
	public:
		PipelineBuilder(RenderContext* InContext);

		PipelineBuilder& WithShader(ShaderHandle InShader);
		PipelineBuilder& PushConstant(uint32_t InSize);
		PipelineBuilder& AddDescriptorSetLayout(DescriptorSetLayoutHandle InLayout);
		PipelineBuilder& ColorAttachment(ImageFormat InFormat);
		PipelineBuilder& DepthAttachment();
		PipelineBuilder& SetPolygonMode(PolygonModeType InPolygonMode);
		Pipeline Build();

	private:
		RenderContext* m_Context = nullptr;
		PipelineInfo m_PipelineInfo = {};
		uint32_t m_PushConstantOffset = 0;
	};

}
