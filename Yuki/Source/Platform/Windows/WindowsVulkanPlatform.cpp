#include "WindowsVulkanPlatform.hpp"
#include "WindowsWindow.hpp"

namespace Yuki {

	void WindowsVulkanPlatform::GetRequiredInstanceExtensions(List<const char*>& InExtensions) const
	{
		InExtensions.EmplaceBack(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	}

	VkSurfaceKHR WindowsVulkanPlatform::CreateSurface(VkInstance InInstance, GenericWindow* InWindow) const
	{
		auto* nativeWindow = static_cast<WindowsWindow*>(InWindow);

		VkWin32SurfaceCreateInfoKHR surfaceInfo = {
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = GetModuleHandle(nullptr),
			.hwnd = nativeWindow->GetWindowHandle(),
		};

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		YUKI_VERIFY(vkCreateWin32SurfaceKHR(InInstance, &surfaceInfo, nullptr, &surface) == VK_SUCCESS);
		return surface;
	}

	Unique<VulkanPlatform> VulkanPlatform::New() { return Unique<WindowsVulkanPlatform>::Create(); }

}
