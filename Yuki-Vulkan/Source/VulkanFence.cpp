#include "VulkanRHI.hpp"

namespace Yuki {

	Fence Fence::Create(RHIContext context)
	{
		auto* impl = new Impl();
		impl->Context = context;

		VkSemaphoreTypeCreateInfo typeInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.pNext = nullptr,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = 0,
		};

		VkSemaphoreCreateInfo semaphoreInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &typeInfo,
			.flags = 0,
		};

		Vulkan::CheckResult(vkCreateSemaphore(context->Device, &semaphoreInfo, nullptr, &impl->Resource));
		return { impl };
	}

	void Fence::Destroy()
	{
		vkDestroySemaphore(m_Impl->Context->Device, m_Impl->Resource, nullptr);
		delete m_Impl;
	}

	uint64_t Fence::GetValue() const { return m_Impl->Value; }
	uint64_t Fence::GetCurrentValue() const
	{
		uint64_t value;
		Vulkan::CheckResult(vkGetSemaphoreCounterValue(m_Impl->Context->Device, m_Impl->Resource, &value));
		return value;
	}

	void Fence::Wait(uint64_t value) const
	{
		VkSemaphoreWaitInfo waitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.pNext = nullptr,
			.flags = 0,
			.semaphoreCount = 1,
			.pSemaphores = &m_Impl->Resource,
			.pValues = value ? &value : &m_Impl->Value,
		};

		vkWaitSemaphores(m_Impl->Context->Device, &waitInfo, std::numeric_limits<uint64_t>::max());
	}

}
