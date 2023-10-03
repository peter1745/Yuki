#include "VulkanPlatformUtils.hpp"

namespace Yuki::Vulkan {

	void AddPlatformInstanceExtensions(DynamicArray<const char*>& InExtensions)
	{
#if defined(YUKI_PLATFORM_WINDOWS)
		InExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
	}

	VkSurfaceKHR CreateSurface(VkInstance InInstance, const WindowSystem& InWindowSystem, WindowHandle InWindowHandle)
	{
		VkSurfaceKHR Result = VK_NULL_HANDLE;

#if defined(YUKI_PLATFORM_WINDOWS)
		VkWin32SurfaceCreateInfoKHR SurfaceInfo =
		{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = GetModuleHandle(nullptr),
			.hwnd = Cast<HWND>(InWindowSystem.GetWindowData(InWindowHandle).NativeHandle),
		};

		YUKI_VERIFY(vkCreateWin32SurfaceKHR(InInstance, &SurfaceInfo, nullptr, &Result) == VK_SUCCESS);
#endif

		return Result;
	}

}
