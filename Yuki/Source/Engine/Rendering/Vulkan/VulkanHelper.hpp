#pragma once

#include "Containers/List.hpp"

namespace Yuki {

	class VulkanHelper
	{
	public:
		template<typename TVulkanType, typename TEnumerationFunc, typename... TArgs>
		static void Enumerate(TEnumerationFunc&& InEnumerationFunc, List<TVulkanType>& InList, TArgs&&... InArgs)
		{
			uint32_t numElements = 0;
			InEnumerationFunc(std::forward<TArgs>(InArgs)..., &numElements, nullptr);
			InList.Resize(size_t(numElements));
			InEnumerationFunc(std::forward<TArgs>(InArgs)..., &numElements, InList.Data());
		}
	};

}
