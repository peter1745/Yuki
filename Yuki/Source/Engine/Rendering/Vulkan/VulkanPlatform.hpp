#pragma once

#include "Core/GenericWindow.hpp"

#include "VulkanDevice.hpp"

namespace Yuki {

	class VulkanPlatform
	{
	public:
		virtual ~VulkanPlatform() = default;

		static void GetRequiredInstanceExtensions(List<const char*>& InExtensions);
		static VkSurfaceKHR CreateSurface(VkInstance InInstance, GenericWindow* InWindow);

		static const List<Unique<VulkanDevice>>& QueryAvailableDevices(VkInstance InInstance);
	};

}
