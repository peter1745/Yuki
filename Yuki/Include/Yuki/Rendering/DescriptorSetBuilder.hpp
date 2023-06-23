#pragma once

#include "RHI.hpp"

namespace Yuki {

	class RenderContext;

	class DescriptorSetLayoutBuilder
	{
	public:
		DescriptorSetLayoutBuilder(RenderContext* InContext);

		DescriptorSetLayoutBuilder& Stages(ShaderStage InStages);
		DescriptorSetLayoutBuilder& Binding(uint32_t InDescriptorCount, DescriptorType InDescriptorType);
		DescriptorSetLayout Build();

	private:
		RenderContext* m_Context = nullptr;
		DescriptorSetLayoutInfo m_Info;
	};

}
