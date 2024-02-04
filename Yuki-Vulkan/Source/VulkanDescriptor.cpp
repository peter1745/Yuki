#include "VulkanRHI.hpp"

namespace Yuki {

	DescriptorHeap DescriptorHeap::Create(RHIContext context)
	{
		auto* impl = new Impl();
		impl->Context = context;

		VkDescriptorSetLayoutCreateFlags bindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		auto bindingFlagsArray = std::array{ bindingFlags, bindingFlags };

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.bindingCount = 2,
			.pBindingFlags = bindingFlagsArray.data(),
		};

		auto bindings = std::array
		{
			VkDescriptorSetLayoutBinding
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.descriptorCount = 1000,
				.stageFlags = VK_SHADER_STAGE_ALL
			},

			VkDescriptorSetLayoutBinding
			{
				.binding = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
				.descriptorCount = 1000,
				.stageFlags = VK_SHADER_STAGE_ALL
			},
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = static_cast<uint32_t>(bindings.size()),
			.pBindings = bindings.data(),
		};
		Vulkan::CheckResult(vkCreateDescriptorSetLayout(context->Device, &layoutInfo, nullptr, &impl->Layout));

		auto poolSizes = std::array
		{
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		};

		VkDescriptorPoolCreateInfo poolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = 1,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data(),
		};
		Vulkan::CheckResult(vkCreateDescriptorPool(context->Device, &poolInfo, nullptr, &impl->Pool));

		VkDescriptorSetAllocateInfo allocateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = impl->Pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &impl->Layout,
		};

		Vulkan::CheckResult(vkAllocateDescriptorSets(
			context->Device,
			&allocateInfo,
			&impl->Set
		));

		return { impl };
	}

	void DescriptorHeap::Destroy()
	{
		vkDestroyDescriptorPool(m_Impl->Context->Device, m_Impl->Pool, nullptr);
		vkDestroyDescriptorSetLayout(m_Impl->Context->Device, m_Impl->Layout, nullptr);
	}

	void DescriptorHeap::WriteSampledImage(uint32_t index, ImageView imageView)
	{
		VkDescriptorImageInfo imageInfo =
		{
			.imageView = imageView->Resource,
			.imageLayout = imageView->Source->Layout,
		};

		VkWriteDescriptorSet writeDescriptor =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_Impl->Set,
			.dstBinding = 0,
			.dstArrayElement = index,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.pImageInfo = &imageInfo,
		};

		vkUpdateDescriptorSets(m_Impl->Context->Device, 1, &writeDescriptor, 0, nullptr);
	}

	void DescriptorHeap::WriteSampler(uint32_t index, Sampler sampler)
	{
		VkDescriptorImageInfo imageInfo =
		{
			.sampler = sampler->Resource
		};

		VkWriteDescriptorSet writeDescriptor =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_Impl->Set,
			.dstBinding = 1,
			.dstArrayElement = index,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.pImageInfo = &imageInfo,
		};

		vkUpdateDescriptorSets(m_Impl->Context->Device, 1, &writeDescriptor, 0, nullptr);
	}
}
