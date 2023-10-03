#pragma once

#include "Engine/Common/WindowSystem.hpp"

#include "VulkanInclude.hpp"

namespace Yuki::Vulkan {

	void AddPlatformInstanceExtensions(DynamicArray<const char*>& InExtensions);
	VkSurfaceKHR CreateSurface(VkInstance InInstance, const WindowSystem& InWindowSystem, WindowHandle InWindowHandle);

}
