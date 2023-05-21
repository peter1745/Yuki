#pragma once

#include "RenderAPI.hpp"
#include "RenderContext.hpp"
#include "GraphicsPipeline.hpp"
#include "ImageFormat.hpp"

namespace Yuki {

	class GraphicsPipelineBuilder
	{
	public:
		virtual ~GraphicsPipelineBuilder() = default;

		virtual GraphicsPipelineBuilder* WithShader(ResourceHandle<Shader> InShaderHandle) = 0;
		virtual GraphicsPipelineBuilder* AddVertexInput(uint32_t InLocation, ShaderDataType InDataType) = 0;
		virtual GraphicsPipelineBuilder* ColorAttachment(ImageFormat InFormat) = 0;
		virtual ResourceHandle<GraphicsPipeline> Build() = 0;

	public:
		static Unique<GraphicsPipelineBuilder> New(RenderAPI InRenderAPI, RenderContext* InContext);
	};

}
