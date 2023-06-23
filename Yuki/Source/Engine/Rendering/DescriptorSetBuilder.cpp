#include "Rendering/DescriptorSetBuilder.hpp"
#include "Rendering/RenderContext.hpp"

namespace Yuki {

	DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(RenderContext* InContext)
		: m_Context(InContext)
	{
	}

	DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::Stages(ShaderStage InStages)
	{
		m_Info.Stages = InStages;
		return *this;
	}

	DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::Binding(uint32_t InDescriptorCount, DescriptorType InDescriptorType)
	{
		m_Info.Descriptors.emplace_back(InDescriptorCount, InDescriptorType);
		return *this;
	}

	DescriptorSetLayout DescriptorSetLayoutBuilder::Build()
	{
		return m_Context->CreateDescriptorSetLayout(m_Info);
	}
}
