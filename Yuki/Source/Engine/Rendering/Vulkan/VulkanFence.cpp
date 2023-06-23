#include "VulkanFence.hpp"
#include "VulkanRenderContext.hpp"

namespace Yuki {

	Fence VulkanRenderContext::CreateFence()
	{
		auto[handle, fence] = m_Fences.Acquire();

		VkSemaphoreTypeCreateInfo semaphoreTypeInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = 0,
		};

		VkSemaphoreCreateInfo semaphoreInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &semaphoreTypeInfo,
		};
		vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &fence.Semaphore);

		return handle;
	}

	void VulkanRenderContext::DestroyFence(Fence InFence)
	{
		auto& fence = m_Fences.Get(InFence);
		vkDestroySemaphore(m_LogicalDevice, fence.Semaphore, nullptr);
		m_Fences.Return(InFence);
	}

	void VulkanRenderContext::FenceWait(Fence InFence, uint64_t InValue)
	{
		auto& fence = m_Fences.Get(InFence);

		VkSemaphoreWaitInfo waitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &fence.Semaphore,
			.pValues = InValue ? &InValue : &fence.Value,
		};
		vkWaitSemaphores(m_LogicalDevice, &waitInfo, UINT64_MAX);
	}

}
