#pragma once

#include <Engine/Common/Types.hpp>

namespace Yuki::Vulkan {

	template<typename TVulkanType, typename TEnumeratorFunc, typename... TFuncArgs>
	inline void Enumerate(TEnumeratorFunc&& InEnumeratorFunc, DynamicArray<TVulkanType>& InArray, TFuncArgs&&... InArgs)
	{
		uint32_t NumElements = 0;
		InEnumeratorFunc(std::forward<TFuncArgs>(InArgs)..., &NumElements, nullptr);
		InArray.resize(static_cast<size_t>(NumElements));
		InEnumeratorFunc(std::forward<TFuncArgs>(InArgs)..., &NumElements, InArray.data());
	}

}
