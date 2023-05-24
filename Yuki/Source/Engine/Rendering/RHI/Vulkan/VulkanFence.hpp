#pragma once

#include "Rendering/RHI/Fence.hpp"

#include "VulkanRenderContext.hpp"

namespace Yuki {

	class VulkanFence : public Fence
	{
	public:
		void Wait(uint64_t InValue = 0) override;

		uint64_t& GetValue() override { return m_Value; }

		VkSemaphore GetVkSemaphore() const { return m_Semaphore; }

	private:
		static VulkanFence* Create(VulkanRenderContext* InContext);
		static void Destroy(VulkanRenderContext* InContext, VulkanFence* InFence);

	private:
		VulkanRenderContext* m_Context = nullptr;
		VkSemaphore m_Semaphore = VK_NULL_HANDLE;
		uint64_t m_Value = 0;

	private:
		friend class VulkanRenderContext;
	};

}
