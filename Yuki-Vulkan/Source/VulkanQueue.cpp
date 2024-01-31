#include "VulkanRHI.hpp"

#include <Aura/Stack.hpp>

namespace Yuki {

	Aura::Span<Queue> RHIContext::RequestQueues(QueueType type, uint32_t count) const
	{
		uint32_t startIndex = ~0;
		uint32_t endIndex = ~0;

		for (uint32_t i = 0; i < m_Impl->Queues.size(); i++)
		{
			auto queue = m_Impl->Queues[i];

			auto typeValue = std::to_underlying(type);
			if ((queue->Flags & typeValue) != typeValue)
			{
				if (startIndex != ~0)
				{
					// If we have a start index then the first non-matching queue we find is
					// one index past the last matching queue
					endIndex = i - 1;
					break;
				}

				continue;
			}

			if (startIndex == ~0)
			{
				startIndex = i;
				endIndex = i;
			}
		}

		YukiAssert(startIndex != ~0 && endIndex != ~0);

		return { &m_Impl->Queues[startIndex], endIndex - startIndex + 1 };
	}

	Queue RHIContext::RequestQueue(QueueType type) const
	{
		uint32_t bestScore = ~0u;
		uint32_t bestQueue = ~0u;

		for (uint32_t i = 0; i < m_Impl->Queues.size(); i++)
		{
			auto queue = m_Impl->Queues[i];

			auto typeValue = std::to_underlying(type);
			if ((queue->Flags & typeValue) != typeValue)
			{
				continue;
			}

			uint32_t score = std::popcount(queue->Flags & typeValue);

			if (score < bestScore)
			{
				bestScore = score;
				bestQueue = i;
			}
		}

		YukiAssert(bestScore != ~0u && bestQueue != ~0u);

		return m_Impl->Queues[bestQueue];
	}

	static VkResult AcquireNextImage(VkDevice device, Swapchain swapchain)
	{
		VkAcquireNextImageInfoKHR acquireImageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
			.swapchain = swapchain->Resource,
			.timeout = std::numeric_limits<uint64_t>::max(),
			.semaphore = swapchain->Semaphores[swapchain->CurrentSemaphoreIndex],
			.fence = nullptr,
			.deviceMask = 1,
		};

		return vkAcquireNextImage2KHR(device, &acquireImageInfo, &swapchain->CurrentImageIndex);
	}

	void Queue::AcquireImages(Aura::Span<Swapchain> swapchains, Aura::Span<Fence> signals) const
	{
		AuraStackPoint();

		if (swapchains.IsEmpty())
		{
			return;
		}

		for (auto swapchain : swapchains)
		{
			if (AcquireNextImage(m_Impl->Context->Device, swapchain) == VK_ERROR_OUT_OF_DATE_KHR)
			{
				swapchain->Recreate();
				Vulkan::CheckResult(AcquireNextImage(m_Impl->Context->Device, swapchain));
			}
		}

		if (!signals.IsEmpty())
		{
			auto waitInfos = Aura::StackAlloc<VkSemaphoreSubmitInfo>(swapchains.Count());

			for (uint32_t i = 0; i < swapchains.Count(); i++)
			{
				auto swapchain = swapchains[i];
				waitInfos[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
				waitInfos[i].semaphore = swapchain->Semaphores[swapchain->CurrentSemaphoreIndex];
				swapchain->CurrentSemaphoreIndex = (swapchain->CurrentSemaphoreIndex + 1) % static_cast<uint32_t>(swapchain->Semaphores.size());
			}

			auto signalInfos = Aura::StackAlloc<VkSemaphoreSubmitInfo>(signals.Count());

			for (uint32_t i = 0; i < signals.Count(); i++)
			{
				signalInfos[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
				signalInfos[i].semaphore = signals[i]->Resource;
				signalInfos[i].value = ++signals[i]->Value;
				signalInfos[i].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			}

			VkSubmitInfo2 submitInfo =
			{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.waitSemaphoreInfoCount = waitInfos.Count(),
				.pWaitSemaphoreInfos = waitInfos.Data(),
				.signalSemaphoreInfoCount = signalInfos.Count(),
				.pSignalSemaphoreInfos = signalInfos.Data(),
			};

			Vulkan::CheckResult(vkQueueSubmit2(m_Impl->Queue, 1, &submitInfo, nullptr));
		}
	}

	void Queue::SubmitCommandLists(Aura::Span<CommandList> commandLists, Aura::Span<Fence> waits, Aura::Span<Fence> signals) const
	{
		AuraStackPoint();

		auto commandListSubmits = Aura::StackAlloc<VkCommandBufferSubmitInfo>(commandLists.Count());
		for (uint32_t i = 0; i < commandLists.Count(); i++)
		{
			vkEndCommandBuffer(commandLists[i]->Resource);

			commandListSubmits[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			commandListSubmits[i].commandBuffer = commandLists[i]->Resource;
		}

		auto waitSubmits = Aura::StackAlloc<VkSemaphoreSubmitInfo>(waits.Count());
		for (uint32_t i = 0; i < waits.Count(); i++)
		{
			waitSubmits[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			waitSubmits[i].semaphore = waits[i]->Resource;
			waitSubmits[i].value = waits[i]->Value;
			waitSubmits[i].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		}

		auto signalSubmits = Aura::StackAlloc<VkSemaphoreSubmitInfo>(signals.Count());
		for (uint32_t i = 0; i < signals.Count(); i++)
		{
			signalSubmits[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			signalSubmits[i].semaphore = signals[i]->Resource;
			signalSubmits[i].value = ++signals[i]->Value;
			signalSubmits[i].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		}

		VkSubmitInfo2 submitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.pNext = nullptr,
			.flags = 0,
			.waitSemaphoreInfoCount = waitSubmits.Count(),
			.pWaitSemaphoreInfos = waitSubmits.Data(),
			.commandBufferInfoCount = commandListSubmits.Count(),
			.pCommandBufferInfos = commandListSubmits.Data(),
			.signalSemaphoreInfoCount = signalSubmits.Count(),
			.pSignalSemaphoreInfos = signalSubmits.Data(),
		};

		Vulkan::CheckResult(vkQueueSubmit2(m_Impl->Queue, 1, &submitInfo, nullptr));
	}

	void Queue::Present(Aura::Span<Swapchain> swapchains, Aura::Span<Fence> waits) const
	{
		AuraStackPoint();

		if (swapchains.IsEmpty())
		{
			return;
		}

		auto presentResults = Aura::StackAlloc<VkResult>(swapchains.Count());
		auto binaryWaits = Aura::StackAlloc<VkSemaphore>(0);

		if (!waits.IsEmpty())
		{
			auto waitInfos = Aura::StackAlloc<VkSemaphoreSubmitInfo>(waits.Count());
			for (uint32_t i = 0; i < waits.Count(); i++)
			{
				waitInfos[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
				waitInfos[i].semaphore = waits[i]->Resource;
				waitInfos[i].value = waits[i]->Value;
				waitInfos[i].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			}

			auto signalInfos = Aura::StackAlloc<VkSemaphoreSubmitInfo>(swapchains.Count());
			for (uint32_t i = 0; i < swapchains.Count(); i++)
			{
				signalInfos[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
				signalInfos[i].semaphore = swapchains[i]->Semaphores[swapchains[i]->CurrentSemaphoreIndex];
			}

			VkSubmitInfo2 submitInfo =
			{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.waitSemaphoreInfoCount = waitInfos.Count(),
				.pWaitSemaphoreInfos = waitInfos.Data(),
				.signalSemaphoreInfoCount = signalInfos.Count(),
				.pSignalSemaphoreInfos = signalInfos.Data(),
			};

			vkQueueSubmit2(m_Impl->Queue, 1, &submitInfo, nullptr);

			binaryWaits = Aura::StackAlloc<VkSemaphore>(swapchains.Count());
			for (uint32_t i = 0; i < swapchains.Count(); i++)
			{
				auto swapchain = swapchains[i];
				binaryWaits[i] = swapchain->Semaphores[swapchain->CurrentSemaphoreIndex];
				swapchain->CurrentSemaphoreIndex = (swapchain->CurrentSemaphoreIndex + 1) % static_cast<uint32_t>(swapchain->Semaphores.size());
			}
		}

		auto swapchainHandles = Aura::StackAlloc<VkSwapchainKHR>(swapchains.Count());
		auto imageIndices = Aura::StackAlloc<uint32_t>(swapchains.Count());
		for (uint32_t i = 0; i < swapchains.Count(); i++)
		{
			swapchainHandles[i] = swapchains[i]->Resource;
			imageIndices[i] = swapchains[i]->CurrentImageIndex;
		}

		VkPresentInfoKHR presentInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = binaryWaits.Count(),
			.pWaitSemaphores = binaryWaits.Data(),
			.swapchainCount = swapchainHandles.Count(),
			.pSwapchains = swapchainHandles.Data(),
			.pImageIndices = imageIndices.Data(),
			.pResults = presentResults.Data(),
		};

		vkQueuePresentKHR(m_Impl->Queue, &presentInfo);

		for (uint32_t i = 0; i < presentResults.Count(); i++)
		{
			if (presentResults[i] != VK_ERROR_OUT_OF_DATE_KHR)
				continue;

			swapchains[i]->Recreate();
		}
	}

}
