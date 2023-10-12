#include "VulkanRHI.hpp"

namespace Yuki::RHI {

	static VkDescriptorType DescriptorTypeToVkDescriptorType(DescriptorType type)
	{
		switch (type)
		{
		case DescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
		case DescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case DescriptorType::SampledImage: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case DescriptorType::StorageImage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case DescriptorType::UniformTexelBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case DescriptorType::StorageTexelBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		case DescriptorType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case DescriptorType::UniformBufferDynamic: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		case DescriptorType::StorageBufferDynamic: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		case DescriptorType::InputAttachment: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		}

		YUKI_VERIFY(false);
		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}

	static VkShaderStageFlags ShaderStagesToVkShaderStageFlags(ShaderStage stages)
	{
		VkShaderStageFlags result = 0;

		if (stages & ShaderStage::Vertex)			result |= VK_SHADER_STAGE_VERTEX_BIT;
		if (stages & ShaderStage::Fragment)			result |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if (stages & ShaderStage::RayGeneration)	result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		if (stages & ShaderStage::RayMiss)			result |= VK_SHADER_STAGE_MISS_BIT_KHR;
		if (stages & ShaderStage::RayClosestHit)	result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		return result;
	}

	DescriptorSetLayout DescriptorSetLayout::Create(Context context, const DescriptorSetLayoutInfo& info)
	{
		auto layout = new Impl();

		DynamicArray<VkDescriptorSetLayoutBinding> bindings(info.Descriptors.size());
		for (uint32_t i = 0; i < info.Descriptors.size(); i++)
		{
			const auto& bindingInfo = info.Descriptors.at(i);
			bindings[i] =
			{
				.binding = i,
				.descriptorType = DescriptorTypeToVkDescriptorType(bindingInfo.Type),
				.descriptorCount = bindingInfo.Count,
				.stageFlags = ShaderStagesToVkShaderStageFlags(info.Stages),
				.pImmutableSamplers = nullptr,
			};
		}

		DynamicArray<VkDescriptorBindingFlags> bindingFlags(info.Descriptors.size(), VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.pNext = nullptr,
			.bindingCount = Cast<uint32_t>(bindingFlags.size()),
			.pBindingFlags = bindingFlags.data(),
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &bindingFlagsInfo,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = Cast<uint32_t>(bindings.size()),
			.pBindings = bindings.data(),
		};

		vkCreateDescriptorSetLayout(context->Device, &layoutInfo, nullptr, &layout->Handle);
		return { layout };
	}

	/*void VulkanRenderDevice::DescriptorSetLayoutDestroy(DescriptorSetLayoutRH InLayout)
	{
		auto& Layout = m_DescriptorSetLayouts[InLayout];
		vkDestroyDescriptorSetLayout(m_Device, Layout.Handle, nullptr);
		m_DescriptorSetLayouts.Return(InLayout);
	}*/

	DescriptorPool DescriptorPool::Create(Context context, Span<DescriptorCount> descriptorCounts)
	{
		auto pool = new Impl();
		pool->Ctx = context;

		DynamicArray<VkDescriptorPoolSize> descriptorSizes;
		descriptorSizes.reserve(descriptorCounts.Count());
		for (const auto& descriptorCount : descriptorCounts)
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
			.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = 1000,
			.poolSizeCount = uint32_t(descriptorSizes.size()),
			.pPoolSizes = descriptorSizes.data(),
		};

		vkCreateDescriptorPool(context->Device, &descriptorPoolInfo, nullptr, &pool->Handle);

		return { pool };
	}

	/*void VulkanRenderDevice::DescriptorPoolDestroy(DescriptorPoolRH InPool)
	{
		auto& Pool = m_DescriptorPools[InPool];
		vkDestroyDescriptorPool(m_Device, Pool.Handle, nullptr);
		m_DescriptorPools.Return(InPool);
	}*/

	DescriptorSet DescriptorPool::AllocateDescriptorSet(DescriptorSetLayoutRH layout)
	{
		auto set = new DescriptorSet::Impl();
		set->Ctx = m_Impl->Ctx;
		set->Layout = layout;

		VkDescriptorSetAllocateInfo allocateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = m_Impl->Handle,
			.descriptorSetCount = 1,
			.pSetLayouts = &layout->Handle,
		};

		vkAllocateDescriptorSets(m_Impl->Ctx->Device, &allocateInfo, &set->Handle);

		DescriptorSet result = { set };
		m_Impl->AllocatedSets.push_back(result);
		return result;
	}

	void DescriptorSet::Write(uint32_t binding, Span<ImageViewRH> imageViews, uint32_t arrayOffset)
	{
		if (imageViews.IsEmpty())
			return;

		DynamicArray<VkDescriptorImageInfo> descriptorImageInfos;
		descriptorImageInfos.reserve(imageViews.Count());

		for (auto view : imageViews)
		{
			auto& descriptorImageInfo = descriptorImageInfos.emplace_back();
			descriptorImageInfo.sampler = VK_NULL_HANDLE;
			descriptorImageInfo.imageView = view->Handle;
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}

		VkWriteDescriptorSet writeDescriptor =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_Impl->Handle,
			.dstBinding = binding,
			.dstArrayElement = arrayOffset,
			.descriptorCount = uint32_t(descriptorImageInfos.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.pImageInfo = descriptorImageInfos.data(),
		};

		vkUpdateDescriptorSets(m_Impl->Ctx->Device, 1, &writeDescriptor, 0, nullptr);
	}

}
