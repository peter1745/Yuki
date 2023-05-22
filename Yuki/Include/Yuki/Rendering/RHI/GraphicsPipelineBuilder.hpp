#pragma once

#include "Yuki/Rendering/RenderAPI.hpp"
#include "Yuki/Rendering/ImageFormat.hpp"

#include "RenderContext.hpp"
#include "GraphicsPipeline.hpp"

#include "Shader.hpp"

namespace Yuki {

	class GraphicsPipelineBuilder
	{
	public:
		virtual ~GraphicsPipelineBuilder() = default;

		virtual GraphicsPipelineBuilder* WithShader(ResourceHandle<Shader> InShaderHandle) = 0;
		virtual GraphicsPipelineBuilder* AddVertexInput(uint32_t InLocation, ShaderDataType InDataType) = 0;
		virtual GraphicsPipelineBuilder* ColorAttachment(ImageFormat InFormat) = 0;
		virtual GraphicsPipelineBuilder* DepthAttachment() = 0;
		virtual Unique<GraphicsPipeline> Build() = 0;

	public:
		static Unique<GraphicsPipelineBuilder> New(RenderAPI InRenderAPI, RenderContext* InContext);
	};

}
