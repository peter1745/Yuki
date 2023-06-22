#pragma once

#include "Core/Core.hpp"
#include "VulkanQueue.hpp"

#include "Rendering/RHI/Image2D.hpp"
#include "Rendering/RHI/DescriptorSet.hpp"
#include "Rendering/RHI/Buffer.hpp"

namespace Yuki {

	class VulkanHelper
	{
	public:
		template<typename TVulkanType, typename TEnumerationFunc, typename... TArgs>
		static void Enumerate(TEnumerationFunc&& InEnumerationFunc, List<TVulkanType>& InList, TArgs&&... InArgs)
		{
			uint32_t numElements = 0;
			InEnumerationFunc(std::forward<TArgs>(InArgs)..., &numElements, nullptr);
			InList.resize(size_t(numElements));
			InEnumerationFunc(std::forward<TArgs>(InArgs)..., &numElements, InList.data());
		}

		static uint32_t SelectGraphicsQueue(VkPhysicalDevice InPhysicalDevice);

		static void TransitionImage(VkCommandBuffer InCommandBuffer, VkImage InImage, VkPipelineStageFlags2 InSrcStage, VkAccessFlags2 InSrcAccess, VkImageLayout InSrcLayout, VkPipelineStageFlags2 InDstStage, VkAccessFlags2 InDstAccess, VkImageLayout InDstLayout, VkImageAspectFlags InAspectFlags);

	public:
		// Conversion Functions
		static VkFormat ImageFormatToVkFormat(ImageFormat InFormat);
		static ImageFormat VkFormatToImageFormat(VkFormat InFormat);

		static VkBufferUsageFlags BufferTypeToVkUsageFlags(BufferType InType);

		static VkImageLayout ImageLayoutToVkImageLayout(ImageLayout InLayout);
		static VkFlags ImageUsageToVkFlags(ImageUsage InUsage);

		static VkDescriptorType DescriptorTypeToVkDescriptorType(DescriptorType InType);
	};

}
