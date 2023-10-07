#include "VulkanRHI.hpp"
#include "VulkanRenderDevice.hpp"

namespace Yuki::RHI {

	Fence Fence::Create(Context InContext)
	{
		auto Fence = new Impl();

		VkSemaphoreTypeCreateInfo SemaphoreTypeInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.pNext = nullptr,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = 0,
		};

		VkSemaphoreCreateInfo SemaphoreInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &SemaphoreTypeInfo
		};

		YUKI_VERIFY(vkCreateSemaphore(InContext->Device, &SemaphoreInfo, nullptr, &Fence->Handle) == VK_SUCCESS);
		return { Fence };
	}

	/*void VulkanRenderDevice::FenceWait(FenceRH InFence, uint64_t InValue)
	{
		auto& Fence = m_Fences[InFence];
		VkSemaphoreWaitInfo WaitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &Fence.Handle,
			.pValues = InValue ? &InValue : &Fence.Value,
		};
		vkWaitSemaphores(m_Device, &WaitInfo, UINT64_MAX);
	}

	void VulkanRenderDevice::FenceDestroy(FenceRH InFence)
	{
		auto& Fence = m_Fences[InFence];
		vkDestroySemaphore(m_Device, Fence.Handle, nullptr);
		m_Fences.Return(InFence);
	}*/
}
