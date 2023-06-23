#pragma once

#include "Core/GenericWindow.hpp"

#include "VulkanInclude.hpp"

namespace Yuki {

	class VulkanPlatform
	{
	public:
		virtual ~VulkanPlatform() = default;

		static void GetRequiredInstanceExtensions(DynamicArray<const char*>& InExtensions);
		static VkSurfaceKHR CreateSurface(VkInstance InInstance, GenericWindow* InWindow);
	};

}
