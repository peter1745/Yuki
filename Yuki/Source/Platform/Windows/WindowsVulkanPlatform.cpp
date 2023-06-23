#include "Engine/Rendering/Vulkan/VulkanPlatform.hpp"
#include "WindowsWindow.hpp"

namespace Yuki {

	void VulkanPlatform::GetRequiredInstanceExtensions(DynamicArray<const char*>& InExtensions)
	{
		InExtensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	}

	VkSurfaceKHR VulkanPlatform::CreateSurface(VkInstance InInstance, GenericWindow* InWindow)
	{
		auto* nativeWindow = static_cast<WindowsWindow*>(InWindow);

		VkWin32SurfaceCreateInfoKHR surfaceInfo =
		{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = GetModuleHandle(nullptr),
			.hwnd = nativeWindow->GetWindowHandle(),
		};

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		YUKI_VERIFY(vkCreateWin32SurfaceKHR(InInstance, &surfaceInfo, nullptr, &surface) == VK_SUCCESS);
		return surface;
	}

}