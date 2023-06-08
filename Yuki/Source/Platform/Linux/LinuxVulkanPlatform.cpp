#include "Engine/Rendering/RHI/Vulkan/VulkanPlatform.hpp"
#include "LinuxWindow.hpp"

namespace Yuki {

	void VulkanPlatform::GetRequiredInstanceExtensions(List<const char*>& InExtensions)
	{
		InExtensions.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
	}

	VkSurfaceKHR VulkanPlatform::CreateSurface(VkInstance InInstance, GenericWindow* InWindow)
	{
		auto* nativeWindow = static_cast<LinuxWindow*>(InWindow);

		VkXcbSurfaceCreateInfoKHR surfaceInfo =
		{
			.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.connection = nativeWindow->GetConnection(),
			.window = nativeWindow->GetWindowHandle(),
		};

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		vkCreateXcbSurfaceKHR(InInstance, &surfaceInfo, nullptr, &surface);
		return surface;
	}

}
