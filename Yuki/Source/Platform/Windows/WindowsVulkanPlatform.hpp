#pragma once

#include "Engine/Rendering/Vulkan/VulkanPlatform.hpp"

namespace Yuki {

	class WindowsVulkanPlatform : public VulkanPlatform
	{
	public:
		void GetRequiredInstanceExtensions(List<const char*>& InExtensions) const override;
		VkSurfaceKHR CreateSurface(VkInstance InInstance, GenericWindow* InWindow) const override;

	};

}
