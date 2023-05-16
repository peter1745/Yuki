#pragma once

#include "Core/GenericWindow.hpp"

#include "VulkanDevice.hpp"

namespace Yuki {

	class VulkanPlatform
	{
	public:
		virtual ~VulkanPlatform() = default;

		virtual void GetRequiredInstanceExtensions(List<const char*>& InExtensions) const = 0;
		virtual VkSurfaceKHR CreateSurface(VkInstance InInstance, GenericWindow* InWindow) const = 0;

		const List<Unique<VulkanDevice>>& QueryAvailableDevices(VkInstance InInstance);

	public:
		static Unique<VulkanPlatform> New();

	private:
		List<Unique<VulkanDevice>> m_AvailableDevices;
	};

}
