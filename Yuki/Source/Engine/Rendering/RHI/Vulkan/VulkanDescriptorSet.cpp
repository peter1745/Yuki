#include "VulkanDescriptorSet.hpp"
#include "VulkanRenderContext.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	VulkanDescriptorPool::VulkanDescriptorPool(VulkanRenderContext* InContext, std::span<DescriptorCount> InDescriptorCounts)
		: m_Context(InContext)
	{
		std::vector<VkDescriptorPoolSize> descriptorSizes;
		descriptorSizes.reserve(InDescriptorCounts.size());
		for (const auto& descriptorCount : InDescriptorCounts)
		{
			VkDescriptorPoolSize size =
			{
				.type = VulkanHelper::DescriptorTypeToVkDescriptorType(descriptorCount.Type),
				.descriptorCount = descriptorCount.Count,
			};
			descriptorSizes.emplace_back(std::move(size));
		}

		VkDescriptorPoolCreateInfo descriptorPoolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = 1000,
			.poolSizeCount = uint32_t(descriptorSizes.size()),
			.pPoolSizes = descriptorSizes.data(),
		};

		vkCreateDescriptorPool(InContext->GetDevice(), &descriptorPoolInfo, nullptr, &m_Pool);
	}

	DescriptorSet* VulkanDescriptorPool::AllocateSet(DescriptorSetLayout* InSetLayout)
	{
		auto* setLayout = static_cast<VulkanDescriptorSetLayout*>(InSetLayout);

		VkDescriptorSetAllocateInfo allocateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = m_Pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &setLayout->Handle,
		};

		VkDescriptorSet descriptorSet;
		vkAllocateDescriptorSets(m_Context->GetDevice(), &allocateInfo, &descriptorSet);

		auto* set = new VulkanDescriptorSet();
		set->m_Context = m_Context;
		set->m_Layout = Unique<VulkanDescriptorSetLayout>(setLayout);
		set->m_Set = descriptorSet;

		return m_Sets.emplace_back(set);
	}

	void VulkanDescriptorSet::Write(uint32_t InBinding, std::span<Image2D* const> InImages, Sampler* InSampler)
	{
		List<VkDescriptorImageInfo> descriptorImageInfos;
		descriptorImageInfos.reserve(InImages.size());

		for (auto* const image : InImages)
		{
			auto& descriptorImageInfo = descriptorImageInfos.emplace_back();
			descriptorImageInfo.sampler = static_cast<VulkanSampler*>(InSampler)->GetVkSampler();
			descriptorImageInfo.imageView = static_cast<VulkanImageView2D*>(image->GetDefaultImageView())->GetVkImageView();
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		VkWriteDescriptorSet writeDescriptor =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_Set,
			.dstBinding = InBinding,
			.dstArrayElement = 0,
			.descriptorCount = uint32_t(descriptorImageInfos.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = descriptorImageInfos.data(),
		};

		vkUpdateDescriptorSets(m_Context->GetDevice(), 1, &writeDescriptor, 0, nullptr);
	}

}
