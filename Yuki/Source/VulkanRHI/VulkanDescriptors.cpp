#include "VulkanRHI.hpp"

namespace Yuki::RHI {

	static VkDescriptorType DescriptorTypeToVkDescriptorType(DescriptorType InType)
	{
		switch (InType)
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

	static VkShaderStageFlags ShaderStagesToVkShaderStageFlags(ShaderStage InStages)
	{
		VkShaderStageFlags Result = 0;

		if (InStages & ShaderStage::Vertex) Result |= VK_SHADER_STAGE_VERTEX_BIT;
		if (InStages & ShaderStage::Fragment) Result |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if (InStages & ShaderStage::RayGeneration) Result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		if (InStages & ShaderStage::RayMiss) Result |= VK_SHADER_STAGE_MISS_BIT_KHR;
		if (InStages & ShaderStage::RayClosestHit) Result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

		return Result;
	}

	DescriptorSetLayout DescriptorSetLayout::Create(Context InContext, const DescriptorSetLayoutInfo& InLayoutInfo)
	{
		auto Layout = new Impl();

		DynamicArray<VkDescriptorSetLayoutBinding> Bindings(InLayoutInfo.Descriptors.size());
		for (uint32_t Index = 0; Index < InLayoutInfo.Descriptors.size(); Index++)
		{
			const auto& BindingInfo = InLayoutInfo.Descriptors.at(Index);
			Bindings[Index] =
			{
				.binding = Index,
				.descriptorType = DescriptorTypeToVkDescriptorType(BindingInfo.Type),
				.descriptorCount = BindingInfo.Count,
				.stageFlags = ShaderStagesToVkShaderStageFlags(InLayoutInfo.Stages),
				.pImmutableSamplers = nullptr,
			};
		}

		DynamicArray<VkDescriptorBindingFlags> BindingFlags(InLayoutInfo.Descriptors.size(), VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
		VkDescriptorSetLayoutBindingFlagsCreateInfo BindingFlagsInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.pNext = nullptr,
			.bindingCount = Cast<uint32_t>(BindingFlags.size()),
			.pBindingFlags = BindingFlags.data(),
		};

		VkDescriptorSetLayoutCreateInfo LayoutInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &BindingFlagsInfo,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.bindingCount = Cast<uint32_t>(Bindings.size()),
			.pBindings = Bindings.data(),
		};

		vkCreateDescriptorSetLayout(InContext->Device, &LayoutInfo, nullptr, &Layout->Handle);
		return { Layout };
	}

	/*void VulkanRenderDevice::DescriptorSetLayoutDestroy(DescriptorSetLayoutRH InLayout)
	{
		auto& Layout = m_DescriptorSetLayouts[InLayout];
		vkDestroyDescriptorSetLayout(m_Device, Layout.Handle, nullptr);
		m_DescriptorSetLayouts.Return(InLayout);
	}*/

	DescriptorPool DescriptorPool::Create(Context InContext, Span<DescriptorCount> InDescriptorCounts)
	{
		auto Pool = new Impl();
		Pool->Ctx = InContext;

		DynamicArray<VkDescriptorPoolSize> DescriptorSizes;
		DescriptorSizes.reserve(InDescriptorCounts.Count());
		for (const auto& DescriptorCount : InDescriptorCounts)
		{
			VkDescriptorPoolSize Size =
			{
				.type = DescriptorTypeToVkDescriptorType(DescriptorCount.Type),
				.descriptorCount = DescriptorCount.Count,
			};
			DescriptorSizes.emplace_back(std::move(Size));
		}

		VkDescriptorPoolCreateInfo DescriptorPoolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = 1000,
			.poolSizeCount = uint32_t(DescriptorSizes.size()),
			.pPoolSizes = DescriptorSizes.data(),
		};

		vkCreateDescriptorPool(InContext->Device, &DescriptorPoolInfo, nullptr, &Pool->Handle);

		return { Pool };
	}

	/*void VulkanRenderDevice::DescriptorPoolDestroy(DescriptorPoolRH InPool)
	{
		auto& Pool = m_DescriptorPools[InPool];
		vkDestroyDescriptorPool(m_Device, Pool.Handle, nullptr);
		m_DescriptorPools.Return(InPool);
	}*/

	DescriptorSet DescriptorPool::AllocateDescriptorSet(DescriptorSetLayoutRH InLayout)
	{
		auto Set = new DescriptorSet::Impl();
		Set->Ctx = m_Impl->Ctx;
		Set->Layout = InLayout;

		VkDescriptorSetAllocateInfo AllocateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = m_Impl->Handle,
			.descriptorSetCount = 1,
			.pSetLayouts = &InLayout->Handle,
		};

		vkAllocateDescriptorSets(m_Impl->Ctx->Device, &AllocateInfo, &Set->Handle);

		DescriptorSet Result = { Set };
		m_Impl->AllocatedSets.push_back(Result);
		return Result;
	}

	void DescriptorSet::Write(uint32_t InBinding, Span<ImageViewRH> InImageViews, uint32_t InArrayOffset)
	{
		if (InImageViews.IsEmpty())
			return;

		DynamicArray<VkDescriptorImageInfo> DescriptorImageInfos;
		DescriptorImageInfos.reserve(InImageViews.Count());

		for (auto View : InImageViews)
		{
			auto& DescriptorImageInfo = DescriptorImageInfos.emplace_back();
			DescriptorImageInfo.sampler = VK_NULL_HANDLE;
			DescriptorImageInfo.imageView = View->Handle;
			DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}

		VkWriteDescriptorSet writeDescriptor =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_Impl->Handle,
			.dstBinding = InBinding,
			.dstArrayElement = InArrayOffset,
			.descriptorCount = uint32_t(DescriptorImageInfos.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.pImageInfo = DescriptorImageInfos.data(),
		};

		vkUpdateDescriptorSets(m_Impl->Ctx->Device, 1, &writeDescriptor, 0, nullptr);
	}

}
