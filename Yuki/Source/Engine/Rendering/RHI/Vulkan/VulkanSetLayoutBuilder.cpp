#include "VulkanSetLayoutBuilder.hpp"
#include "VulkanRenderContext.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	static VkShaderStageFlags ShaderStagesToVkShaderStageFlags(ShaderStage InStages)
	{
		VkShaderStageFlags result = 0;

		if (InStages & ShaderStage::Vertex) result |= VK_SHADER_STAGE_VERTEX_BIT;
		if (InStages & ShaderStage::Fragment) result |= VK_SHADER_STAGE_FRAGMENT_BIT;

		return result;
	}

	VulkanSetLayoutBuilder::VulkanSetLayoutBuilder(VulkanRenderContext* InContext)
		: m_Context(InContext)
	{
	}

	SetLayoutBuilder& VulkanSetLayoutBuilder::Start()
	{
		m_Stages = ShaderStage::None;
		m_Bindings.clear();
		return *this;
	}

	SetLayoutBuilder& VulkanSetLayoutBuilder::Stages(ShaderStage InStages)
	{
		m_Stages = InStages;
		return *this;
	}

	SetLayoutBuilder& VulkanSetLayoutBuilder::Binding(uint32_t InDescriptorCount, DescriptorType InType)
	{
		VkDescriptorSetLayoutBinding binding =
		{
			.binding = uint32_t(m_Bindings.size()),
			.descriptorType = VulkanHelper::DescriptorTypeToVkDescriptorType(InType),
			.descriptorCount = InDescriptorCount,
			.stageFlags = ShaderStagesToVkShaderStageFlags(m_Stages),
			.pImmutableSamplers = nullptr,
		};

		m_Bindings.emplace_back(std::move(binding));
		return *this;
	}

	DescriptorSetLayout* VulkanSetLayoutBuilder::Build()
	{
		std::vector<VkDescriptorBindingFlags> bindingFlags(m_Bindings.size(), VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.pNext = nullptr,
			.bindingCount = uint32_t(m_Bindings.size()),
			.pBindingFlags = bindingFlags.data(),
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &bindingFlagsInfo,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = uint32_t(m_Bindings.size()),
			.pBindings = m_Bindings.data(),
		};

		VkDescriptorSetLayout layout;
		vkCreateDescriptorSetLayout(m_Context->GetDevice(), &layoutInfo, nullptr, &layout);

		VulkanDescriptorSetLayout* result = new VulkanDescriptorSetLayout();
		result->Handle = layout;
		return result;
	}

}
