#include "VulkanDescriptorPool.hpp"
#include "VulkanRenderContext.hpp"

namespace Yuki {

	static VkDescriptorType DescriptorTypeToVkDescriptorType(DescriptorType InType)
	{
		switch (InType)
		{
		case DescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case DescriptorType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		}

		YUKI_VERIFY(false);
		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
	
	static VkShaderStageFlags ShaderStagesToVkShaderStageFlags(ShaderStage InStages)
	{
		VkShaderStageFlags result = 0;

		if (InStages & ShaderStage::Vertex) result |= VK_SHADER_STAGE_VERTEX_BIT;
		if (InStages & ShaderStage::Fragment) result |= VK_SHADER_STAGE_FRAGMENT_BIT;

		return result;
	}

	DescriptorSetLayoutHandle VulkanRenderContext::CreateDescriptorSetLayout(const DescriptorSetLayoutInfo& InLayoutInfo)
	{
		auto[handle, layout] = m_DescriptorSetLayouts.Acquire();

		DynamicArray<VkDescriptorSetLayoutBinding> bindings(InLayoutInfo.Descriptors.size());
		for (size_t i = 0; i < InLayoutInfo.Descriptors.size(); i++)
		{
			const auto& bindingInfo = InLayoutInfo.Descriptors.at(i);
			bindings[i] =
			{
				.binding = uint32_t(i),
				.descriptorType = DescriptorTypeToVkDescriptorType(bindingInfo.Type),
				.descriptorCount = bindingInfo.Count,
				.stageFlags = ShaderStagesToVkShaderStageFlags(InLayoutInfo.Stages),
				.pImmutableSamplers = nullptr,
			};
		}

		DynamicArray<VkDescriptorBindingFlags> bindingFlags(InLayoutInfo.Descriptors.size(), VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.pNext = nullptr,
			.bindingCount = uint32_t(bindingFlags.size()),
			.pBindingFlags = bindingFlags.data(),
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &bindingFlagsInfo,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = uint32_t(bindings.size()),
			.pBindings = bindings.data(),
		};

		vkCreateDescriptorSetLayout(m_LogicalDevice, &layoutInfo, nullptr, &layout.Handle);
		return handle;
	}

	void VulkanRenderContext::Destroy(DescriptorSetLayoutHandle InLayout)
	{
		auto& layout = m_DescriptorSetLayouts.Get(InLayout);
		vkDestroyDescriptorSetLayout(m_LogicalDevice, layout.Handle, nullptr);
		m_DescriptorSetLayouts.Return(InLayout);
	}

	DescriptorPoolHandle VulkanRenderContext::CreateDescriptorPool(std::span<DescriptorCount> InDescriptorCounts)
	{
		auto[handle, pool] = m_DescriptorPools.Acquire();

		DynamicArray<VkDescriptorPoolSize> descriptorSizes;
		descriptorSizes.reserve(InDescriptorCounts.size());
		for (const auto& descriptorCount : InDescriptorCounts)
		{
			VkDescriptorPoolSize size =
			{
				.type = DescriptorTypeToVkDescriptorType(descriptorCount.Type),
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

		vkCreateDescriptorPool(m_LogicalDevice, &descriptorPoolInfo, nullptr, &pool.Handle);

		return handle;
	}

	void VulkanRenderContext::Destroy(DescriptorPoolHandle InPool)
	{
		auto& pool = m_DescriptorPools.Get(InPool);
		vkDestroyDescriptorPool(m_LogicalDevice, pool.Handle, nullptr);
		m_DescriptorPools.Return(InPool);
	}

	DescriptorSetHandle VulkanRenderContext::DescriptorPoolAllocateDescriptorSet(DescriptorPoolHandle InPool, DescriptorSetLayoutHandle InLayout)
	{
		auto& pool = m_DescriptorPools.Get(InPool);
		auto& layout = m_DescriptorSetLayouts.Get(InLayout);

		auto[handle, set] = m_DescriptorSets.Acquire();
		set.Layout = InLayout;

		VkDescriptorSetAllocateInfo allocateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = pool.Handle,
			.descriptorSetCount = 1,
			.pSetLayouts = &layout.Handle,
		};

		vkAllocateDescriptorSets(m_LogicalDevice, &allocateInfo, &set.Handle);

		pool.Sets.emplace_back(handle);
		return handle;
	}

	void VulkanRenderContext::DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageHandle> InImages, SamplerHandle InSampler, uint32_t InArrayOffset)
	{
		DynamicArray<ImageViewHandle> imageViews;
		imageViews.reserve(InImages.size());
		for (auto imageHandle : InImages)
			imageViews.emplace_back(m_Images.Get(imageHandle).DefaultImageView);
		DescriptorSetWrite(InSet, InBinding, imageViews, InSampler, InArrayOffset);
	}

	void VulkanRenderContext::DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageHandle> InImages, std::span<SamplerHandle> InSamplers, uint32_t InArrayOffset)
	{
		DynamicArray<ImageViewHandle> imageViews;
		imageViews.reserve(InImages.size());
		for (auto imageHandle : InImages)
			imageViews.emplace_back(m_Images.Get(imageHandle).DefaultImageView);
		DescriptorSetWrite(InSet, InBinding, imageViews, InSamplers, InArrayOffset);
	}

	void VulkanRenderContext::DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageViewHandle> InImageViews, SamplerHandle InSampler, uint32_t InArrayOffset)
	{
		if (InImageViews.empty())
			return;

		auto& set = m_DescriptorSets.Get(InSet);

		DynamicArray<VkDescriptorImageInfo> descriptorImageInfos;
		descriptorImageInfos.reserve(InImageViews.size());

		auto& sampler = m_Samplers.Get(InSampler);

		for (auto imageViewHandle : InImageViews)
		{
			auto& imageView = m_ImageViews.Get(imageViewHandle);
			auto& descriptorImageInfo = descriptorImageInfos.emplace_back();
			descriptorImageInfo.sampler = sampler.Handle;
			descriptorImageInfo.imageView = imageView.ImageView;
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		VkWriteDescriptorSet writeDescriptor =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = set.Handle,
			.dstBinding = InBinding,
			.dstArrayElement = InArrayOffset,
			.descriptorCount = uint32_t(descriptorImageInfos.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = descriptorImageInfos.data(),
		};

		vkUpdateDescriptorSets(m_LogicalDevice, 1, &writeDescriptor, 0, nullptr);
	}

	void VulkanRenderContext::DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<ImageViewHandle> InImageViews, std::span<SamplerHandle> InSamplers, uint32_t InArrayOffset)
	{
		if (InImageViews.empty())
			return;

		auto& set = m_DescriptorSets.Get(InSet);

		DynamicArray<VkDescriptorImageInfo> descriptorImageInfos;
		descriptorImageInfos.reserve(InImageViews.size());

		for (size_t i = 0; i < InImageViews.size(); i++)
		{
			auto& imageView = m_ImageViews.Get(InImageViews[i]);
			auto& sampler = m_Samplers.Get(InSamplers[i]);
			auto& descriptorImageInfo = descriptorImageInfos.emplace_back();
			descriptorImageInfo.sampler = sampler.Handle;
			descriptorImageInfo.imageView = imageView.ImageView;
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		VkWriteDescriptorSet writeDescriptor =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = set.Handle,
			.dstBinding = InBinding,
			.dstArrayElement = InArrayOffset,
			.descriptorCount = uint32_t(descriptorImageInfos.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = descriptorImageInfos.data(),
		};

		vkUpdateDescriptorSets(m_LogicalDevice, 1, &writeDescriptor, 0, nullptr);
	}

	void VulkanRenderContext::DescriptorSetWrite(DescriptorSetHandle InSet, uint32_t InBinding, std::span<std::pair<uint32_t, BufferHandle>> InBuffers, uint32_t InArrayOffset)
	{
		auto& set = m_DescriptorSets.Get(InSet);

		DynamicArray<VkDescriptorBufferInfo> bufferDescriptorInfos(InBuffers.size());
		DynamicArray<VkWriteDescriptorSet> bufferWrites(InBuffers.size());

		for (size_t i = 0; i < InBuffers.size(); i++)
		{
			auto& buffer = m_Buffers.Get(InBuffers[i].second);

			bufferDescriptorInfos[i] =
			{
				.buffer = buffer.Handle,
				.offset = 0,
				.range = VK_WHOLE_SIZE,
			};

			bufferWrites[i] =
			{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.pNext = nullptr,
				.dstSet = set.Handle,
				.dstBinding = InBuffers[i].first,
				.dstArrayElement = InArrayOffset,
				.descriptorCount = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.pBufferInfo = &bufferDescriptorInfos[i],
			};
		}

		vkUpdateDescriptorSets(m_LogicalDevice, uint32_t(bufferWrites.size()), bufferWrites.data(), 0, nullptr);
	}

}
