#include "VulkanPlatform.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	const List<Unique<VulkanDevice>>& VulkanPlatform::QueryAvailableDevices(VkInstance InInstance)
	{
		if (m_AvailableDevices.IsEmpty())
		{
			List<VkPhysicalDevice> availablePhysicalDevices;
			VulkanHelper::Enumerate(vkEnumeratePhysicalDevices, availablePhysicalDevices, InInstance);
			
			m_AvailableDevices.Resize(availablePhysicalDevices.Count());

			for (size_t i = 0; i < availablePhysicalDevices.Count(); i++)
				m_AvailableDevices[i] = std::move(Unique<VulkanDevice>::Create(availablePhysicalDevices[i]));
		}

		return m_AvailableDevices;
	}

}
