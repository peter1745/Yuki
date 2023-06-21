#pragma once

#include "Rendering/RHI/Queue.hpp"

#include "VulkanInclude.hpp"

namespace Yuki {

	class VulkanRenderContext;
	class VulkanCommandBufferPool;

	class VulkanQueue : public Queue
	{
	public:
		void SubmitCommandBuffers(const InitializerList<CommandBuffer*>& InCommandBuffers, const InitializerList<Fence*> InWaits, const InitializerList<Fence*> InSignals) override;
		void SubmitCommandBuffers(std::span<VkCommandBuffer const> InCommandBuffers, const InitializerList<Fence*> InWaits, const InitializerList<Fence*> InSignals);

		void AcquireImages(std::span<Viewport* const> InViewports, const InitializerList<Fence*> InFences) override;
		void Present(std::span<Viewport* const> InViewports, const InitializerList<Fence*> InFences) override;

		void WaitIdle() const override;

		uint32_t GetFamilyIndex() const { return m_QueueFamily; }

	private:
		VulkanRenderContext* m_Context = nullptr;
		VkQueue m_Queue = VK_NULL_HANDLE;
		uint32_t m_QueueFamily = 0;

		VulkanCommandBufferPool* m_PresentTransitionPool = nullptr;

		friend class VulkanRenderContext;
	};

}
