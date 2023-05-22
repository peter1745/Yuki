#pragma once

#include "Core/Core.hpp"
#include "VulkanQueue.hpp"

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
	};

}
