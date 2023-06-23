#pragma once

#include "Rendering/RHI.hpp"

#include "VulkanInclude.hpp"

namespace Yuki {

	class VulkanHelper
	{
	public:
		template<typename TVulkanType, typename TEnumerationFunc, typename... TArgs>
		static void Enumerate(TEnumerationFunc&& InEnumerationFunc, DynamicArray<TVulkanType>& InList, TArgs&&... InArgs)
		{
			uint32_t numElements = 0;
			InEnumerationFunc(std::forward<TArgs>(InArgs)..., &numElements, nullptr);
			InList.resize(size_t(numElements));
			InEnumerationFunc(std::forward<TArgs>(InArgs)..., &numElements, InList.data());
		}

	public:
		static VkFormat ImageFormatToVkFormat(ImageFormat InFormat);
		static VkImageLayout ImageLayoutToVkImageLayout(ImageLayout InLayout);
		static ImageLayout VkImageLayoutToImageLayout(VkImageLayout InLayout);
	};

}
