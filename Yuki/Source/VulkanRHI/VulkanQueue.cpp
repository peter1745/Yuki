#include "VulkanQueue.hpp"
#include "VulkanRenderDevice.hpp"
#include "VulkanSwapchain.hpp"

namespace Yuki::RHI {

	QueueRH VulkanRenderDevice::QueueRequest(QueueType InType)
	{
		uint32_t BestScore = std::numeric_limits<uint32_t>::max();
		QueueRH BestQueue = {};

		m_Queues.ForEach([InType, &BestScore, &BestQueue](const auto& Handle, const auto& Queue)
		{
			VkQueueFlags Flags = std::to_underlying(InType);
			if ((Queue.Flags & Flags) != Flags)
				return;

			uint32_t Score = std::popcount(Queue.Flags & ~Cast<uint32_t>(Flags));

			if (Score < BestScore)
			{
				BestScore = Score;
				BestQueue = Handle;
			}
		});

		return BestQueue;
	}

	static VkResult AcquireNextImage(VkDevice InLogicalDevice, VulkanSwapchain& InSwapchain)
	{
		VkAcquireNextImageInfoKHR acquireImageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
			.swapchain = InSwapchain.Handle,
			.timeout = UINT64_MAX,
			.semaphore = InSwapchain.Semaphores[InSwapchain.CurrentSemaphoreIndex],
			.fence = VK_NULL_HANDLE,
			.deviceMask = 1
		};

		return vkAcquireNextImage2KHR(InLogicalDevice, &acquireImageInfo, &InSwapchain.CurrentImageIndex);
	}

	void VulkanRenderDevice::QueueAcquireImages(QueueRH InQueue, Span<SwapchainRH> InSwapchains, Span<FenceRH> InFences)
	{
		if (InSwapchains.IsEmpty())
			return;

		for (auto SwapchainHandle : InSwapchains)
		{
			auto& Swapchain = m_Swapchains[SwapchainHandle];

			if (AcquireNextImage(m_Device, Swapchain) == VK_ERROR_OUT_OF_DATE_KHR)
			{
				SwapchainRecreate(Swapchain);
				YUKI_VERIFY(AcquireNextImage(m_Device, Swapchain) == VK_SUCCESS);
			}
		}

		if (!InFences.IsEmpty())
		{
			DynamicArray<VkSemaphoreSubmitInfo> WaitInfos;
			WaitInfos.resize(InSwapchains.Count());
			for (size_t Index = 0; Index < InSwapchains.Count(); Index++)
			{
				auto& Swapchain = m_Swapchains.Get(InSwapchains[Index]);

				WaitInfos[Index] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = Swapchain.Semaphores[Swapchain.CurrentSemaphoreIndex],
				};

				Swapchain.CurrentSemaphoreIndex = (Swapchain.CurrentSemaphoreIndex + 1) % uint32_t(Swapchain.Semaphores.size());
			}

			DynamicArray<VkSemaphoreSubmitInfo> SignalInfos;
			SignalInfos.resize(InFences.Count());
			for (size_t Index = 0; Index < InFences.Count(); Index++)
			{
				auto& Fence = m_Fences.Get(InFences[Index]);

				SignalInfos[Index] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = Fence.Handle,
					.value = ++Fence.Value,
					.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
				};
			}

			VkSubmitInfo2 SubmitInfo =
			{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.waitSemaphoreInfoCount = uint32_t(WaitInfos.size()),
				.pWaitSemaphoreInfos = WaitInfos.data(),
				.signalSemaphoreInfoCount = uint32_t(SignalInfos.size()),
				.pSignalSemaphoreInfos = SignalInfos.data()
			};
			YUKI_VERIFY(vkQueueSubmit2(m_Queues[InQueue].Handle, 1, &SubmitInfo, VK_NULL_HANDLE) == VK_SUCCESS);
		}
	}

	void VulkanRenderDevice::QueueSubmit(QueueRH InQueue, Span<CommandListRH> InCommandLists, Span<FenceRH> InWaits, Span<FenceRH> InSignals)
	{
		auto& Queue = m_Queues[InQueue];

		DynamicArray<VkCommandBufferSubmitInfo> CommandListSubmitInfos;
		CommandListSubmitInfos.resize(InCommandLists.Count());
		for (size_t Index = 0; Index < InCommandLists.Count(); Index++)
		{
			CommandListSubmitInfos[Index].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			CommandListSubmitInfos[Index].commandBuffer = m_CommandLists[InCommandLists[Index]].Handle;
		}

		DynamicArray<VkSemaphoreSubmitInfo> WaitSemaphores;
		WaitSemaphores.resize(InWaits.Count());
		for (size_t Index = 0; Index < InWaits.Count(); Index++)
		{
			auto& Fence = m_Fences[InWaits[Index]];
			WaitSemaphores[Index] =
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = Fence.Handle,
				.value = Fence.Value,
				.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			};
		}

		DynamicArray<VkSemaphoreSubmitInfo> SignalSemaphores;
		SignalSemaphores.resize(InSignals.Count());
		for (size_t Index = 0; Index < InSignals.Count(); Index++)
		{
			auto& Fence = m_Fences.Get(InSignals[Index]);

			SignalSemaphores[Index] =
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = Fence.Handle,
				.value = ++Fence.Value,
				.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			};
		}

		VkSubmitInfo2 SubmitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = uint32_t(WaitSemaphores.size()),
			.pWaitSemaphoreInfos = WaitSemaphores.data(),
			.commandBufferInfoCount = uint32_t(CommandListSubmitInfos.size()),
			.pCommandBufferInfos = CommandListSubmitInfos.data(),
			.signalSemaphoreInfoCount = uint32_t(SignalSemaphores.size()),
			.pSignalSemaphoreInfos = SignalSemaphores.data(),
		};

		YUKI_VERIFY(vkQueueSubmit2(Queue.Handle, 1, &SubmitInfo, VK_NULL_HANDLE) == VK_SUCCESS);
	}

	void VulkanRenderDevice::QueuePresent(QueueRH InQueue, Span<SwapchainRH> InSwapchains, Span<FenceRH> InFences)
	{
		if (InSwapchains.IsEmpty())
			return;

		auto& Queue = m_Queues.Get(InQueue);

		std::vector<VkResult> PresentResults(InSwapchains.Count());
		std::vector<VkSemaphore> BinaryWaits;

		if (!InFences.IsEmpty())
		{
			std::vector<VkSemaphoreSubmitInfo> WaitInfos;
			WaitInfos.resize(InFences.Count());

			for (size_t Index = 0; Index < InFences.Count(); Index++)
			{
				auto& Fence = m_Fences.Get(InFences[Index]);

				WaitInfos[Index] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = Fence.Handle,
					.value = Fence.Value,
					.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
				};
			}

			std::vector<VkSemaphoreSubmitInfo> SignalInfos;
			SignalInfos.resize(InSwapchains.Count());
			for (size_t Index = 0; Index < InSwapchains.Count(); Index++)
			{
				auto& Swapchain = m_Swapchains.Get(InSwapchains[Index]);
				SignalInfos[Index] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = Swapchain.Semaphores[Swapchain.CurrentSemaphoreIndex],
				};
			}

			VkSubmitInfo2 SubmitInfo =
			{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.waitSemaphoreInfoCount = uint32_t(WaitInfos.size()),
				.pWaitSemaphoreInfos = WaitInfos.data(),
				.signalSemaphoreInfoCount = uint32_t(SignalInfos.size()),
				.pSignalSemaphoreInfos = SignalInfos.data()
			};
			vkQueueSubmit2(Queue.Handle, 1, &SubmitInfo, VK_NULL_HANDLE);

			BinaryWaits.resize(InSwapchains.Count());
			for (size_t Index = 0; Index < InSwapchains.Count(); Index++)
			{
				auto& swapchain = m_Swapchains.Get(InSwapchains[Index]);
				BinaryWaits[Index] = swapchain.Semaphores[swapchain.CurrentSemaphoreIndex];
				swapchain.CurrentSemaphoreIndex = (swapchain.CurrentSemaphoreIndex + 1) % uint32_t(swapchain.Semaphores.size());
			}
		}

		std::vector<VkSwapchainKHR> Swapchains;
		std::vector<uint32_t> ImageIndices;
		Swapchains.resize(InSwapchains.Count());
		ImageIndices.resize(InSwapchains.Count());
		for (size_t Index = 0; Index < InSwapchains.Count(); Index++)
		{
			auto& swapchain = m_Swapchains.Get(InSwapchains[Index]);
			Swapchains[Index] = swapchain.Handle;
			ImageIndices[Index] = swapchain.CurrentImageIndex;
		}

		VkPresentInfoKHR PresentInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = uint32_t(BinaryWaits.size()),
			.pWaitSemaphores = BinaryWaits.data(),
			.swapchainCount = uint32_t(Swapchains.size()),
			.pSwapchains = Swapchains.data(),
			.pImageIndices = ImageIndices.data(),
			.pResults = PresentResults.data(),
		};
		vkQueuePresentKHR(Queue.Handle, &PresentInfo);

		for (size_t Index = 0; Index < PresentResults.size(); Index++)
		{
			if (PresentResults[Index] != VK_ERROR_OUT_OF_DATE_KHR)
				continue;

			SwapchainRecreate(m_Swapchains.Get(InSwapchains[Index]));
		}
	}

}
