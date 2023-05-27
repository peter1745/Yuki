#pragma once

#include "Core/GenericWindow.hpp"

#include "Vulkan.hpp"

namespace Yuki {

	class VulkanPlatform
	{
	public:
		virtual ~VulkanPlatform() = default;

		static void GetRequiredInstanceExtensions(List<const char*>& InExtensions);
		static VkSurfaceKHR CreateSurface(VkInstance InInstance, GenericWindow* InWindow);
	};

}