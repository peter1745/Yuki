#include "VulkanRHI.hpp"

namespace Yuki::RHI {

	Fence Fence::Create(Context context)
	{
		auto fence = new Impl();
		fence->Ctx = context;

		VkSemaphoreTypeCreateInfo semaphoreTypeInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.pNext = nullptr,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = 0,
		};

		VkSemaphoreCreateInfo semaphoreInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &semaphoreTypeInfo
		};

		YUKI_VK_CHECK(vkCreateSemaphore(context->Device, &semaphoreInfo, nullptr, &fence->Handle));
		return { fence };
	}

	void Fence::Destroy()
	{
		vkDestroySemaphore(m_Impl->Ctx->Device, m_Impl->Handle, nullptr);
		delete m_Impl;
	}

	void Fence::Wait(uint64_t value)
	{
		VkSemaphoreWaitInfo waitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &m_Impl->Handle,
			.pValues = value ? &value : &m_Impl->Value,
		};
		vkWaitSemaphores(m_Impl->Ctx->Device, &waitInfo, UINT64_MAX);
	}
}
