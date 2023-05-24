#include "VulkanFence.hpp"

namespace Yuki {

	void VulkanFence::Wait(uint64_t InValue)
	{
		VkSemaphoreWaitInfo waitInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &m_Semaphore,
			.pValues = InValue ? &InValue : &m_Value,
		};
		vkWaitSemaphores(m_Context->GetDevice(), &waitInfo, UINT64_MAX);
	}

	VulkanFence* VulkanFence::Create(VulkanRenderContext* InContext)
	{
		VulkanFence* fence = new VulkanFence();
		fence->m_Context = InContext;

		VkSemaphoreTypeCreateInfo semaphoreTypeInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = 0,
		};

		VkSemaphoreCreateInfo semaphoreInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &semaphoreTypeInfo,
		};
		vkCreateSemaphore(InContext->GetDevice(), &semaphoreInfo, nullptr, &fence->m_Semaphore);

		return fence;
	}

	void VulkanFence::Destroy(VulkanRenderContext* InContext, VulkanFence* InFence)
	{
		vkDestroySemaphore(InContext->GetDevice(), InFence->m_Semaphore, nullptr);
	}

}
