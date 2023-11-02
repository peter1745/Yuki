#include "VulkanRHI.hpp"

#include "Engine/Containers/IndexFreeList.hpp"

namespace Yuki::RHI {

	Queue Context::RequestQueue(QueueType type) const
	{
		uint32_t bestScore = std::numeric_limits<uint32_t>::max();
		QueueRH bestQueue = {};

		for (auto queue : m_Impl->Queues)
		{
			VkQueueFlags flags = std::to_underlying(type);
			if ((queue->Flags & flags) != flags)
				continue;

			uint32_t score = std::popcount(queue->Flags & ~Cast<uint32_t>(flags));

			if (score < bestScore)
			{
				bestScore = score;
				bestQueue = queue;
			}
		}

		return { bestQueue };
	}

	DynamicArray<Queue> Context::RequestQueues(QueueType type, uint32_t count) const
	{
		DynamicArray<Queue> result;
		result.reserve(Cast<size_t>(count));

		FixedIndexFreeList freeQueues(m_Impl->Queues.size());

		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t bestScore = std::numeric_limits<uint32_t>::max();
			uint32_t bestQueueIndex = std::numeric_limits<uint32_t>::max();

			for (uint32_t j = 0; j < m_Impl->Queues.size(); j++)
			{
				if (!freeQueues.IsFree(j))
					continue;

				const auto& queue = m_Impl->Queues[j];

				VkQueueFlags flags = std::to_underlying(type);
				if ((queue->Flags & flags) != flags)
					continue;

				uint32_t score = std::popcount(queue->Flags & ~Cast<uint32_t>(flags));

				if (score < bestScore)
				{
					if (bestQueueIndex != std::numeric_limits<uint32_t>::max())
						freeQueues.Return(bestQueueIndex);

					freeQueues.Acquire(j);

					bestScore = score;
					bestQueueIndex = j;
				}
			}

			if (bestQueueIndex != std::numeric_limits<uint32_t>::max())
				result.push_back({ m_Impl->Queues[bestQueueIndex] });
		}

		return result;
	}

	static VkResult AcquireNextImage(VkDevice device, Swapchain swapchain)
	{
		VkAcquireNextImageInfoKHR acquireImageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
			.swapchain = swapchain->Handle,
			.timeout = UINT64_MAX,
			.semaphore = swapchain->Semaphores[swapchain->CurrentSemaphoreIndex],
			.fence = VK_NULL_HANDLE,
			.deviceMask = 1
		};

		return vkAcquireNextImage2KHR(device, &acquireImageInfo, &swapchain->CurrentImageIndex);
	}

	void Queue::AcquireImages(Span<Swapchain> swapchains, Span<Fence> fences) const
	{
		if (swapchains.IsEmpty())
			return;

		for (auto swapchainHandle : swapchains)
		{
			if (AcquireNextImage(m_Impl->Ctx->Device, swapchainHandle) == VK_ERROR_OUT_OF_DATE_KHR)
			{
				swapchainHandle.Recreate();
				YUKI_VK_CHECK(AcquireNextImage(m_Impl->Ctx->Device, swapchainHandle));
			}
		}

		if (!fences.IsEmpty())
		{
			DynamicArray<VkSemaphoreSubmitInfo> waitInfos;
			waitInfos.resize(swapchains.Count());
			for (size_t i = 0; i < swapchains.Count(); i++)
			{
				auto swapchain = swapchains[i];

				waitInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = swapchain->Semaphores[swapchain->CurrentSemaphoreIndex],
				};

				swapchain->CurrentSemaphoreIndex = (swapchain->CurrentSemaphoreIndex + 1) % Cast<uint32_t>(swapchain->Semaphores.size());
			}

			DynamicArray<VkSemaphoreSubmitInfo> signalInfos;
			signalInfos.resize(fences.Count());
			for (size_t i = 0; i < fences.Count(); i++)
			{
				auto fence = fences[i];

				signalInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = fence->Handle,
					.value = ++fence->Value,
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
			YUKI_VK_CHECK(vkQueueSubmit2(m_Impl->Handle, 1, &submitInfo, VK_NULL_HANDLE));
		}
	}

	void Queue::Submit(Span<CommandList> commandLists, Span<Fence> waits, Span<Fence> signals) const
	{
		DynamicArray<VkCommandBufferSubmitInfo> commandListSubmitInfos;
		commandListSubmitInfos.resize(commandLists.Count());
		for (size_t i = 0; i < commandLists.Count(); i++)
		{
			commandListSubmitInfos[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			commandListSubmitInfos[i].commandBuffer = commandLists[i]->Handle;
		}

		DynamicArray<VkSemaphoreSubmitInfo> waitSemaphores;
		waitSemaphores.resize(waits.Count());
		for (size_t i = 0; i < waits.Count(); i++)
		{
			auto& fence = waits[i];
			waitSemaphores[i] =
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = fence->Handle,
				.value = fence->Value,
				.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			};
		}

		DynamicArray<VkSemaphoreSubmitInfo> signalSemaphores;
		signalSemaphores.resize(signals.Count());
		for (size_t i = 0; i < signals.Count(); i++)
		{
			auto& fence = signals[i];

			signalSemaphores[i] =
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = fence->Handle,
				.value = ++fence->Value,
				.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			};
		}

		VkSubmitInfo2 submitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = Cast<uint32_t>(waitSemaphores.size()),
			.pWaitSemaphoreInfos = waitSemaphores.data(),
			.commandBufferInfoCount = Cast<uint32_t>(commandListSubmitInfos.size()),
			.pCommandBufferInfos = commandListSubmitInfos.data(),
			.signalSemaphoreInfoCount = Cast<uint32_t>(signalSemaphores.size()),
			.pSignalSemaphoreInfos = signalSemaphores.data(),
		};

		YUKI_VK_CHECK(vkQueueSubmit2(m_Impl->Handle, 1, &submitInfo, VK_NULL_HANDLE));
	}

	void Queue::Present(Span<Swapchain> swapchains, Span<Fence> fences) const
	{
		if (swapchains.IsEmpty())
			return;

		std::vector<VkResult> presentResults(swapchains.Count());
		std::vector<VkSemaphore> binaryWaits;

		if (!fences.IsEmpty())
		{
			std::vector<VkSemaphoreSubmitInfo> waitInfos;
			waitInfos.resize(fences.Count());

			for (size_t i = 0; i < fences.Count(); i++)
			{
				auto fence = fences[i];

				waitInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = fence->Handle,
					.value = fence->Value,
					.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
				};
			}

			std::vector<VkSemaphoreSubmitInfo> signalInfos;
			signalInfos.resize(swapchains.Count());
			for (size_t i = 0; i < swapchains.Count(); i++)
			{
				auto swapchain = swapchains[i];
				signalInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = swapchain->Semaphores[swapchain->CurrentSemaphoreIndex],
				};
			}

			VkSubmitInfo2 submitInfo =
			{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.waitSemaphoreInfoCount = Cast<uint32_t>(waitInfos.size()),
				.pWaitSemaphoreInfos = waitInfos.data(),
				.signalSemaphoreInfoCount = Cast<uint32_t>(signalInfos.size()),
				.pSignalSemaphoreInfos = signalInfos.data()
			};
			vkQueueSubmit2(m_Impl->Handle, 1, &submitInfo, VK_NULL_HANDLE);

			binaryWaits.resize(swapchains.Count());
			for (size_t i = 0; i < swapchains.Count(); i++)
			{
				auto swapchain = swapchains[i];
				binaryWaits[i] = swapchain->Semaphores[swapchain->CurrentSemaphoreIndex];
				swapchain->CurrentSemaphoreIndex = (swapchain->CurrentSemaphoreIndex + 1) % Cast<uint32_t>(swapchain->Semaphores.size());
			}
		}

		std::vector<VkSwapchainKHR> swapchainHandles;
		std::vector<uint32_t> imageIndices;
		swapchainHandles.resize(swapchains.Count());
		imageIndices.resize(swapchains.Count());
		for (size_t i = 0; i < swapchains.Count(); i++)
		{
			auto swapchain = swapchains[i];
			swapchainHandles[i] = swapchain->Handle;
			imageIndices[i] = swapchain->CurrentImageIndex;
		}

		VkPresentInfoKHR presentInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = Cast<uint32_t>(binaryWaits.size()),
			.pWaitSemaphores = binaryWaits.data(),
			.swapchainCount = Cast<uint32_t>(swapchainHandles.size()),
			.pSwapchains = swapchainHandles.data(),
			.pImageIndices = imageIndices.data(),
			.pResults = presentResults.data(),
		};
		vkQueuePresentKHR(m_Impl->Handle, &presentInfo);

		for (size_t i = 0; i < presentResults.size(); i++)
		{
			if (presentResults[i] != VK_ERROR_OUT_OF_DATE_KHR)
				continue;

			swapchains[i].Recreate();
		}
	}

}
