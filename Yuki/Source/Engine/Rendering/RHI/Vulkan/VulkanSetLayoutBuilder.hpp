#pragma once

#include "Rendering/RHI/SetLayoutBuilder.hpp"

#include "VulkanDescriptorSet.hpp"

namespace Yuki {

	class VulkanRenderContext;

	class VulkanSetLayoutBuilder : public SetLayoutBuilder
	{
	public:
		SetLayoutBuilder& Start() override;
		SetLayoutBuilder& Stages(ShaderStage InStages) override;
		SetLayoutBuilder& Binding(uint32_t InDescriptorCount, DescriptorType InType) override;
		DescriptorSetLayout* Build() override;

	private:
		VulkanSetLayoutBuilder(VulkanRenderContext* InContext);

	private:
		VulkanRenderContext* m_Context;
		ShaderStage m_Stages;
		std::vector<VkDescriptorSetLayoutBinding> m_Bindings;

		friend class VulkanRenderContext;
	};

}
