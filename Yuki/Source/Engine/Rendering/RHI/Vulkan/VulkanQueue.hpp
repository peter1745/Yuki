#pragma once

#include "Rendering/RHI/Queue.hpp"

#include "Vulkan.hpp"

namespace Yuki {

	class VulkanRenderContext;

	class VulkanQueue : public Queue
	{
	public:
		void SubmitCommandBuffers(const InitializerList<CommandBuffer>& InCommandBuffers, const InitializerList<Fence*> InWaits, const InitializerList<Fence*> InSignals) override;

		void Present(std::span<Viewport* const> InViewports, const InitializerList<Fence*> InFences) override;
		void AcquireImages(std::span<Viewport* const> InViewports, const InitializerList<Fence*> InFences) override;

		uint32_t GetFamilyIndex() const { return m_QueueFamily; }

	private:
		VulkanRenderContext* m_Context = nullptr;
		VkQueue m_Queue = VK_NULL_HANDLE;
		uint32_t m_QueueFamily = 0;

		friend class VulkanRenderContext;
	};

}
