#pragma once

#include <Engine/Common/Types.hpp>
#include <Engine/RHI/RenderHandles.hpp>

namespace Yuki::Vulkan {

	template<typename TVulkanType, typename TEnumeratorFunc, typename... TFuncArgs>
	inline void Enumerate(TEnumeratorFunc&& InEnumeratorFunc, DynamicArray<TVulkanType>& InArray, TFuncArgs&&... InArgs)
	{
		uint32_t NumElements = 0;
		InEnumeratorFunc(std::forward<TFuncArgs>(InArgs)..., &NumElements, nullptr);
		InArray.resize(static_cast<size_t>(NumElements));
		InEnumeratorFunc(std::forward<TFuncArgs>(InArgs)..., &NumElements, InArray.data());
	}

	inline VkFormat ImageFormatToVkFormat(RHI::ImageFormat InFormat)
	{
		switch (InFormat)
		{
		case Yuki::RHI::ImageFormat::RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;
		case Yuki::RHI::ImageFormat::BGRA8: return VK_FORMAT_B8G8R8A8_UNORM;
		case Yuki::RHI::ImageFormat::D32SFloat: return VK_FORMAT_D32_SFLOAT;
		}

		return VK_FORMAT_UNDEFINED;
	}

}
