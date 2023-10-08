#include "VulkanRHI.hpp"

namespace Yuki::RHI {

	Fence Fence::Create(Context InContext)
	{
		auto Fence = new Impl();
		Fence->Ctx = InContext;

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

	void Fence::Destroy()
	{
		vkDestroySemaphore(m_Impl->Ctx->Device, m_Impl->Handle, nullptr);
		delete m_Impl;
	}

	void Fence::Wait(uint64_t InValue)
	{
		VkSemaphoreWaitInfo WaitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &m_Impl->Handle,
			.pValues = InValue ? &InValue : &m_Impl->Value,
		};
		vkWaitSemaphores(m_Impl->Ctx->Device, &WaitInfo, UINT64_MAX);
	}
}
