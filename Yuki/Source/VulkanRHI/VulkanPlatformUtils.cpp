#include "VulkanPlatformUtils.hpp"
#include "VulkanUtils.hpp"

namespace Yuki::Vulkan {

	void AddPlatformInstanceExtensions(DynamicArray<const char*>& extensions)
	{
#if defined(YUKI_PLATFORM_WINDOWS)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
	}

	VkSurfaceKHR CreateSurface(VkInstance instance, const WindowSystem& windowSystem, WindowHandle windowHandle)
	{
		VkSurfaceKHR result = VK_NULL_HANDLE;

#if defined(YUKI_PLATFORM_WINDOWS)
		VkWin32SurfaceCreateInfoKHR surfaceInfo =
		{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = GetModuleHandle(nullptr),
			.hwnd = Cast<HWND>(windowSystem.GetWindowData(windowHandle).NativeHandle),
		};

		YUKI_VK_CHECK(vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &result));
#endif

		return result;
	}

}
