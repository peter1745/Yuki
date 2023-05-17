#include "VulkanPlatform.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	static List<Unique<VulkanDevice>> s_AvailableDevices;

	const List<Unique<VulkanDevice>>& VulkanPlatform::QueryAvailableDevices(VkInstance InInstance)
	{
		if (s_AvailableDevices.empty())
		{
			List<VkPhysicalDevice> availablePhysicalDevices;
			VulkanHelper::Enumerate(vkEnumeratePhysicalDevices, availablePhysicalDevices, InInstance);
			
			s_AvailableDevices.resize(availablePhysicalDevices.size());

			for (size_t i = 0; i < availablePhysicalDevices.size(); i++)
				s_AvailableDevices[i] = std::move(Unique<VulkanDevice>::Create(availablePhysicalDevices[i]));
		}

		return s_AvailableDevices;
	}

}
