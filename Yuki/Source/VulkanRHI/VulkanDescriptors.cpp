#include "VulkanDescriptors.hpp"
#include "VulkanRenderDevice.hpp"

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

	DescriptorSetLayoutRH VulkanRenderDevice::DescriptorSetLayoutCreate(const DescriptorSetLayoutInfo& InLayoutInfo)
	{
		auto [Handle, Layout] = m_DescriptorSetLayouts.Acquire();

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

		vkCreateDescriptorSetLayout(m_Device, &LayoutInfo, nullptr, &Layout.Handle);
		return Handle;
	}

	void VulkanRenderDevice::DescriptorSetLayoutDestroy(DescriptorSetLayoutRH InLayout)
	{
		auto& Layout = m_DescriptorSetLayouts[InLayout];
		vkDestroyDescriptorSetLayout(m_Device, Layout.Handle, nullptr);
		m_DescriptorSetLayouts.Return(InLayout);
	}

	DescriptorPoolRH VulkanRenderDevice::DescriptorPoolCreate(Span<DescriptorCount> InDescriptorCounts)
	{
		auto [Handle, Pool] = m_DescriptorPools.Acquire();

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

		vkCreateDescriptorPool(m_Device, &DescriptorPoolInfo, nullptr, &Pool.Handle);

		return Handle;
	}

	void VulkanRenderDevice::DescriptorPoolDestroy(DescriptorPoolRH InPool)
	{
		auto& Pool = m_DescriptorPools[InPool];
		vkDestroyDescriptorPool(m_Device, Pool.Handle, nullptr);
		m_DescriptorPools.Return(InPool);
	}

	DescriptorSetRH VulkanRenderDevice::DescriptorPoolAllocateDescriptorSet(DescriptorPoolRH InPool, DescriptorSetLayoutRH InLayout)
	{
		auto& Pool = m_DescriptorPools[InPool];
		auto& Layout = m_DescriptorSetLayouts[InLayout];

		auto [Handle, Set] = m_DescriptorSets.Acquire();
		Set.Layout = InLayout;

		VkDescriptorSetAllocateInfo AllocateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorPool = Pool.Handle,
			.descriptorSetCount = 1,
			.pSetLayouts = &Layout.Handle,
		};

		vkAllocateDescriptorSets(m_Device, &AllocateInfo, &Set.Handle);

		Pool.AllocatedSets.emplace_back(Handle);
		return Handle;
	}

	void VulkanRenderDevice::DescriptorSetWrite(DescriptorSetRH InSet, uint32_t InBinding, Span<ImageViewRH> InImageViews, uint32_t InArrayOffset)
	{
		if (InImageViews.IsEmpty())
			return;

		auto& Set = m_DescriptorSets[InSet];

		DynamicArray<VkDescriptorImageInfo> DescriptorImageInfos;
		DescriptorImageInfos.reserve(InImageViews.Count());

		for (auto imageViewHandle : InImageViews)
		{
			auto& ImageView = m_ImageViews.Get(imageViewHandle);
			auto& DescriptorImageInfo = DescriptorImageInfos.emplace_back();
			DescriptorImageInfo.sampler = VK_NULL_HANDLE;
			DescriptorImageInfo.imageView = ImageView.Handle;
			DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}

		VkWriteDescriptorSet writeDescriptor =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = Set.Handle,
			.dstBinding = InBinding,
			.dstArrayElement = InArrayOffset,
			.descriptorCount = uint32_t(DescriptorImageInfos.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.pImageInfo = DescriptorImageInfos.data(),
		};

		vkUpdateDescriptorSets(m_Device, 1, &writeDescriptor, 0, nullptr);
	}

}
