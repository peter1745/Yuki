#pragma once

#include "Engine/Common/WindowSystem.hpp"

#include "VulkanInclude.hpp"

namespace Yuki::Vulkan {

	void AddPlatformInstanceExtensions(DynamicArray<const char*>& extensions);
	VkSurfaceKHR CreateSurface(VkInstance instance, const WindowSystem& windowSystem, WindowHandle windowHandle);

}
