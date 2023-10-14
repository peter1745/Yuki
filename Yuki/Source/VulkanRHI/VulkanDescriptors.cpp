#include "VulkanRHI.hpp"

namespace Yuki::RHI {

	DescriptorHeap DescriptorHeap::Create(Context context, uint32_t numDescriptors)
	{
		auto heap = new Impl();
		heap->Ctx = context;
		heap->NumDescriptors = numDescriptors;

		VkMutableDescriptorTypeListEXT mutableDescriptorList =
		{
			.descriptorTypeCount = 3,
			.pDescriptorTypes = std::array {
				VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				VK_DESCRIPTOR_TYPE_SAMPLER
			}.data()
		};

		VkMutableDescriptorTypeCreateInfoEXT mutableDescriptorInfo =
		{
			.sType = VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT,
			.pNext = nullptr,
			.mutableDescriptorTypeListCount = 1,
			.pMutableDescriptorTypeLists = &mutableDescriptorList,
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = &mutableDescriptorInfo,
			.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = 1000,
			.poolSizeCount = 1,
			.pPoolSizes = std::array {
				// TODO(Peter): Might need to specify all used descriptor types here on non-NVIDIA GPUs
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, numDescriptors }
			}.data(),
		};

		vkCreateDescriptorPool(context->Device, &descriptorPoolInfo, nullptr, &heap->Handle);

		VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescriptorAllocInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
			.pNext = nullptr,
			.descriptorSetCount = 1,
			.pDescriptorCounts = &numDescriptors,
		};

		VkDescriptorSetAllocateInfo allocateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = &variableDescriptorAllocInfo,
			.descriptorPool = heap->Handle,
			.descriptorSetCount = 1,
			.pSetLayouts = &context->DescriptorHeapLayout,
		};
		vkAllocateDescriptorSets(context->Device, &allocateInfo, &heap->Set);

		return { heap };
	}

	void DescriptorHeap::WriteStorageImages(uint32_t startIndex, Span<ImageView> storageImages) const
	{
		if (storageImages.IsEmpty())
			return;

		DynamicArray<VkDescriptorImageInfo> descriptorImageInfos;
		descriptorImageInfos.reserve(storageImages.Count());

		for (auto view : storageImages)
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
			.dstSet = m_Impl->Set,
			.dstBinding = 0,
			.dstArrayElement = startIndex,
			.descriptorCount = uint32_t(descriptorImageInfos.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.pImageInfo = descriptorImageInfos.data(),
		};

		vkUpdateDescriptorSets(m_Impl->Ctx->Device, 1, &writeDescriptor, 0, nullptr);
	}

	void DescriptorHeap::WriteSampledImages(uint32_t startIndex, Span<ImageView> sampledImages) const
	{
		if (sampledImages.IsEmpty())
			return;

		DynamicArray<VkDescriptorImageInfo> descriptorImageInfos;
		descriptorImageInfos.reserve(sampledImages.Count());

		for (auto view : sampledImages)
		{
			auto& descriptorImageInfo = descriptorImageInfos.emplace_back();
			descriptorImageInfo.sampler = VK_NULL_HANDLE;
			descriptorImageInfo.imageView = view->Handle;
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		VkWriteDescriptorSet writeDescriptor =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_Impl->Set,
			.dstBinding = 0,
			.dstArrayElement = startIndex,
			.descriptorCount = uint32_t(descriptorImageInfos.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.pImageInfo = descriptorImageInfos.data(),
		};

		vkUpdateDescriptorSets(m_Impl->Ctx->Device, 1, &writeDescriptor, 0, nullptr);
	}

	void DescriptorHeap::WriteSamplers(uint32_t startIndex, Span<Sampler> samplers) const
	{
		if (samplers.IsEmpty())
			return;

		DynamicArray<VkDescriptorImageInfo> descriptorImageInfos;
		descriptorImageInfos.reserve(samplers.Count());

		for (auto sampler : samplers)
		{
			auto& descriptorImageInfo = descriptorImageInfos.emplace_back();
			descriptorImageInfo.sampler = sampler->Handle;
			descriptorImageInfo.imageView = VK_NULL_HANDLE;
			descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		VkWriteDescriptorSet writeDescriptor =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = m_Impl->Set,
			.dstBinding = 0,
			.dstArrayElement = startIndex,
			.descriptorCount = uint32_t(descriptorImageInfos.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.pImageInfo = descriptorImageInfos.data(),
		};

		vkUpdateDescriptorSets(m_Impl->Ctx->Device, 1, &writeDescriptor, 0, nullptr);
	}

	void CommandList::BindDescriptorHeap(PipelineLayout layout, PipelineBindPoint bindPoint, DescriptorHeap heap)
	{
		VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;

		switch (bindPoint)
		{
		case PipelineBindPoint::Graphics:
			pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			break;
		case PipelineBindPoint::RayTracing:
			pipelineBindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
			break;
		}

		vkCmdBindDescriptorSets(m_Impl->Handle, pipelineBindPoint, layout->Handle, 0, 1, &heap->Set, 0, nullptr);
	}

}
