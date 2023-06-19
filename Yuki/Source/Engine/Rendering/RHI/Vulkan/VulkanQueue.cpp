#include "VulkanQueue.hpp"
#include "VulkanRenderContext.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanFence.hpp"

namespace Yuki {

	void VulkanQueue::SubmitCommandBuffers(const InitializerList<CommandBuffer*>& InCommandBuffers, const InitializerList<Fence*> InWaits, const InitializerList<Fence*> InSignals)
	{
		std::vector<VkCommandBuffer> commandBuffers(InCommandBuffers.Size());
		for (size_t i = 0; i < InCommandBuffers.Size(); i++)
			commandBuffers[i] = InCommandBuffers[i]->As<VkCommandBuffer>();
		SubmitCommandBuffers(commandBuffers, InWaits, InSignals);
	}

	void VulkanQueue::SubmitCommandBuffers(std::span<VkCommandBuffer const> InCommandBuffers, const InitializerList<Fence*> InWaits, const InitializerList<Fence*> InSignals)
	{
		std::vector<VkCommandBufferSubmitInfo> commandBufferSubmitInfos;
		commandBufferSubmitInfos.resize(InCommandBuffers.size());
		for (size_t i = 0; i < InCommandBuffers.size(); i++)
		{
			commandBufferSubmitInfos[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			commandBufferSubmitInfos[i].commandBuffer = InCommandBuffers[i];
		}

		std::vector<VkSemaphoreSubmitInfo> waitSemaphores;
		waitSemaphores.resize(InWaits.Size());
		for (size_t i = 0; i < InWaits.Size(); i++)
		{
			waitSemaphores[i] =
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = static_cast<VulkanFence*>(InWaits[i])->GetVkSemaphore(),
				.value = InWaits[i]->GetValue(),
				.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			};
		}

		std::vector<VkSemaphoreSubmitInfo> signalSemaphores;
		signalSemaphores.resize(InSignals.Size());
		for (size_t i = 0; i < InSignals.Size(); i++)
		{
			auto& value = InSignals[i]->GetValue();

			signalSemaphores[i] =
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = static_cast<VulkanFence*>(InSignals[i])->GetVkSemaphore(),
				.value = ++value,
				.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			};
		}

		VkSubmitInfo2 submitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = uint32_t(waitSemaphores.size()),
			.pWaitSemaphoreInfos = waitSemaphores.data(),
			.commandBufferInfoCount = uint32_t(commandBufferSubmitInfos.size()),
			.pCommandBufferInfos = commandBufferSubmitInfos.data(),
			.signalSemaphoreInfoCount = uint32_t(signalSemaphores.size()),
			.pSignalSemaphoreInfos = signalSemaphores.data(),
		};

		vkQueueSubmit2(m_Queue, 1, &submitInfo, VK_NULL_HANDLE);
	}

	void VulkanQueue::Present(std::span<Viewport* const> InViewports, const InitializerList<Fence*> InFences)
	{
		if (InViewports.empty())
			return;

		// Transition images to present
		{
			std::vector<VkCommandBuffer> transitionCommandBuffers(InViewports.size());

			for (size_t i = 0; i < InViewports.size(); i++)
			{
				auto* swapchain = static_cast<VulkanSwapchain*>(InViewports[i]->GetSwapchain());

				transitionCommandBuffers[i] = m_Context->CreateTransientCommandBuffer();

				VulkanImageTransition imageTransition =
				{
					.DstPipelineStage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
					.DstAccessFlags = 0,
					.DstImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				};

				static_cast<VulkanImage2D*>(swapchain->GetCurrentImage())->Transition(transitionCommandBuffers[i], imageTransition);

				vkEndCommandBuffer(transitionCommandBuffers[i]);
			}

			SubmitCommandBuffers(transitionCommandBuffers, InFences, {});
		}

		std::vector<VkResult> presentResults(InViewports.size());
		std::vector<VkSemaphore> binaryWaits;

		if (!InFences.Empty())
		{
			std::vector<VkSemaphoreSubmitInfo> waitInfos;
			waitInfos.resize(InFences.Size());

			for (size_t i = 0; i < InFences.Size(); i++)
			{
				waitInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = static_cast<VulkanFence*>(InFences[i])->GetVkSemaphore(),
					.value = InFences[i]->GetValue(),
					.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
				};
			}

			std::vector<VkSemaphoreSubmitInfo> signalInfos;
			signalInfos.resize(InViewports.size());
			for (size_t i = 0; i < InViewports.size(); i++)
			{
				auto* swapchain = static_cast<VulkanSwapchain*>(InViewports[i]->GetSwapchain());
				signalInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = swapchain->m_Semaphores[swapchain->m_SemaphoreIndex],
				};
			}

			VkSubmitInfo2 submitInfo =
			{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.waitSemaphoreInfoCount = uint32_t(waitInfos.size()),
				.pWaitSemaphoreInfos = waitInfos.data(),
				.signalSemaphoreInfoCount = uint32_t(signalInfos.size()),
				.pSignalSemaphoreInfos = signalInfos.data()
			};
			vkQueueSubmit2(m_Queue, 1, &submitInfo, VK_NULL_HANDLE);

			binaryWaits.resize(InViewports.size());
			for (size_t i = 0; i < InViewports.size(); i++)
			{
				auto* swapchain = static_cast<VulkanSwapchain*>(InViewports[i]->GetSwapchain());
				binaryWaits[i] = swapchain->m_Semaphores[swapchain->m_SemaphoreIndex];
				swapchain->m_SemaphoreIndex = (swapchain->m_SemaphoreIndex + 1) % uint32_t(swapchain->m_Semaphores.size());
			}
		}

		std::vector<VkSwapchainKHR> swapchains;
		std::vector<uint32_t> imageIndices;
		swapchains.resize(InViewports.size());
		imageIndices.resize(InViewports.size());
		for (size_t i = 0; i < InViewports.size(); i++)
		{
			auto* swapchain = static_cast<VulkanSwapchain*>(InViewports[i]->GetSwapchain());
			swapchains[i] = swapchain->m_Swapchain;
			imageIndices[i] = swapchain->m_CurrentImageIndex;
		}

		VkPresentInfoKHR presentInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = uint32_t(binaryWaits.size()),
			.pWaitSemaphores = binaryWaits.data(),
			.swapchainCount = uint32_t(swapchains.size()),
			.pSwapchains = swapchains.data(),
			.pImageIndices = imageIndices.data(),
			.pResults = presentResults.data(),
		};
		vkQueuePresentKHR(m_Queue, &presentInfo);

		for (size_t i = 0; i < presentResults.size(); i++)
		{
			if (presentResults[i] != VK_ERROR_OUT_OF_DATE_KHR)
				continue;

			InViewports[i]->RecreateSwapchain();
		}
	}

	void VulkanQueue::WaitIdle() const
	{
		vkQueueWaitIdle(m_Queue);
	}

	void VulkanQueue::AcquireImages(std::span<Viewport* const> InViewports, const InitializerList<Fence*> InFences)
	{
		if (InViewports.empty())
			return;
		
		for (auto* const viewport : InViewports)
			viewport->AcquireNextImage();

		if (!InFences.Empty())
		{
			std::vector<VkSemaphoreSubmitInfo> waitInfos;
			waitInfos.resize(InViewports.size());
			for (size_t i = 0; i < InViewports.size(); i++)
			{
				auto* swapchain = static_cast<VulkanSwapchain*>(InViewports[i]->GetSwapchain());

				waitInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = swapchain->m_Semaphores[swapchain->m_SemaphoreIndex],
				};

				swapchain->m_SemaphoreIndex = (swapchain->m_SemaphoreIndex + 1) % uint32_t(swapchain->m_Semaphores.size());
			}

			std::vector<VkSemaphoreSubmitInfo> signalInfos;
			signalInfos.resize(InFences.Size());
			for (size_t i = 0; i < InFences.Size(); i++)
			{
				auto& value = InFences[i]->GetValue();

				signalInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = static_cast<VulkanFence*>(InFences[i])->GetVkSemaphore(),
					.value = ++value,
					.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
				};
			}

			VkSubmitInfo2 submitInfo =
			{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.waitSemaphoreInfoCount = uint32_t(waitInfos.size()),
				.pWaitSemaphoreInfos = waitInfos.data(),
				.signalSemaphoreInfoCount = uint32_t(signalInfos.size()),
				.pSignalSemaphoreInfos = signalInfos.data()
			};
			vkQueueSubmit2(m_Queue, 1, &submitInfo, VK_NULL_HANDLE);
		}
	}

}
