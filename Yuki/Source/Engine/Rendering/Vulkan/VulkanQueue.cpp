#include "VulkanQueue.hpp"
#include "VulkanRenderContext.hpp"

namespace Yuki {

	void VulkanRenderContext::QueueSubmitCommandLists(const InitializerList<CommandList>& InCommandLists, const InitializerList<Fence> InWaits, const InitializerList<Fence> InSignals)
	{
		DynamicArray<VkCommandBufferSubmitInfo> commandBufferSubmitInfos;
		commandBufferSubmitInfos.resize(InCommandLists.Size());
		for (size_t i = 0; i < InCommandLists.Size(); i++)
		{
			commandBufferSubmitInfos[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			commandBufferSubmitInfos[i].commandBuffer = m_CommandLists.Get(InCommandLists[i]).CommandBuffer;
		}

		std::vector<VkSemaphoreSubmitInfo> waitSemaphores;
		waitSemaphores.resize(InWaits.Size());
		for (size_t i = 0; i < InWaits.Size(); i++)
		{
			auto& fence = m_Fences.Get(InWaits[i]);
			waitSemaphores[i] =
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = fence.Semaphore,
				.value = fence.Value,
				.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
			};
		}

		std::vector<VkSemaphoreSubmitInfo> signalSemaphores;
		signalSemaphores.resize(InSignals.Size());
		for (size_t i = 0; i < InSignals.Size(); i++)
		{
			auto& fence = m_Fences.Get(InSignals[i]);

			signalSemaphores[i] =
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
				.semaphore = fence.Semaphore,
				.value = ++fence.Value,
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

		vkQueueSubmit2(m_Queues.Get(m_GraphicsQueue).Queue, 1, &submitInfo, VK_NULL_HANDLE);
	}

	static VkResult AcquireNextImage(VkDevice InLogicalDevice, VulkanSwapchain& InSwapchain)
	{
		VkAcquireNextImageInfoKHR acquireImageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
			.swapchain = InSwapchain.Swapchain,
			.timeout = UINT64_MAX,
			.semaphore = InSwapchain.Semaphores[InSwapchain.SemaphoreIndex],
			.fence = VK_NULL_HANDLE,
			.deviceMask = 1
		};

		return vkAcquireNextImage2KHR(InLogicalDevice, &acquireImageInfo, &InSwapchain.CurrentImage);
	}

	void VulkanRenderContext::QueueAcquireImages(std::span<Swapchain> InSwapchains, const InitializerList<Fence>& InFences)
	{
		if (InSwapchains.empty())
			return;

		for (auto swapchainHandle : InSwapchains)
		{
			auto& swapchain = m_Swapchains.Get(swapchainHandle);

			if (AcquireNextImage(m_LogicalDevice, swapchain) == VK_ERROR_OUT_OF_DATE_KHR)
			{
				RecreateSwapchain(swapchain);
				YUKI_VERIFY(AcquireNextImage(m_LogicalDevice, swapchain) == VK_SUCCESS);
			}
		}

		if (!InFences.Empty())
		{
			DynamicArray<VkSemaphoreSubmitInfo> waitInfos;
			waitInfos.resize(InSwapchains.size());
			for (size_t i = 0; i < InSwapchains.size(); i++)
			{
				auto& swapchain = m_Swapchains.Get(InSwapchains[i]);

				waitInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = swapchain.Semaphores[swapchain.SemaphoreIndex],
				};

				swapchain.SemaphoreIndex = (swapchain.SemaphoreIndex + 1) % uint32_t(swapchain.Semaphores.size());
			}

			std::vector<VkSemaphoreSubmitInfo> signalInfos;
			signalInfos.resize(InFences.Size());
			for (size_t i = 0; i < InFences.Size(); i++)
			{
				auto& fence = m_Fences.Get(InFences[i]);

				signalInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = fence.Semaphore,
					.value = ++fence.Value,
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
			vkQueueSubmit2(m_Queues.Get(m_GraphicsQueue).Queue, 1, &submitInfo, VK_NULL_HANDLE);

			if (m_CommandPools.IsValid(m_PresentTransitionPool))
				CommandPoolReset(m_PresentTransitionPool);
			else
				m_PresentTransitionPool = CreateCommandPool();
		}
	}

	void VulkanRenderContext::QueuePresent(std::span<Swapchain> InSwapchains, const InitializerList<Fence>& InFences)
	{
		if (InSwapchains.empty())
			return;

		// Transition images to present
		{
			auto commandListHandle = CreateCommandList(m_PresentTransitionPool);
			CommandListBegin(commandListHandle);

			for (auto swapchainHandle : InSwapchains)
			{
				auto& swapchain = m_Swapchains.Get(swapchainHandle);
				CommandListTransitionImage(commandListHandle, swapchain.Images[swapchain.CurrentImage], ImageLayout::Present);
			}

			CommandListEnd(commandListHandle);
			QueueSubmitCommandLists({ commandListHandle }, {}, InFences);
		}

		std::vector<VkResult> presentResults(InSwapchains.size());
		std::vector<VkSemaphore> binaryWaits;

		if (!InFences.Empty())
		{
			std::vector<VkSemaphoreSubmitInfo> waitInfos;
			waitInfos.resize(InFences.Size());

			for (size_t i = 0; i < InFences.Size(); i++)
			{
				auto& fence = m_Fences.Get(InFences[i]);

				waitInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = fence.Semaphore,
					.value = fence.Value,
					.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
				};
			}

			std::vector<VkSemaphoreSubmitInfo> signalInfos;
			signalInfos.resize(InSwapchains.size());
			for (size_t i = 0; i < InSwapchains.size(); i++)
			{
				auto& swapchain = m_Swapchains.Get(InSwapchains[i]);
				signalInfos[i] =
				{
					.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
					.semaphore = swapchain.Semaphores[swapchain.SemaphoreIndex],
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
			vkQueueSubmit2(m_Queues.Get(m_GraphicsQueue).Queue, 1, &submitInfo, VK_NULL_HANDLE);

			binaryWaits.resize(InSwapchains.size());
			for (size_t i = 0; i < InSwapchains.size(); i++)
			{
				auto& swapchain = m_Swapchains.Get(InSwapchains[i]);
				binaryWaits[i] = swapchain.Semaphores[swapchain.SemaphoreIndex];
				swapchain.SemaphoreIndex = (swapchain.SemaphoreIndex + 1) % uint32_t(swapchain.Semaphores.size());
			}
		}

		std::vector<VkSwapchainKHR> swapchains;
		std::vector<uint32_t> imageIndices;
		swapchains.resize(InSwapchains.size());
		imageIndices.resize(InSwapchains.size());
		for (size_t i = 0; i < InSwapchains.size(); i++)
		{
			auto& swapchain = m_Swapchains.Get(InSwapchains[i]);
			swapchains[i] = swapchain.Swapchain;
			imageIndices[i] = swapchain.CurrentImage;
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
		vkQueuePresentKHR(m_Queues.Get(m_GraphicsQueue).Queue, &presentInfo);

		for (size_t i = 0; i < presentResults.size(); i++)
		{
			if (presentResults[i] != VK_ERROR_OUT_OF_DATE_KHR)
				continue;

			RecreateSwapchain(m_Swapchains.Get(InSwapchains[i]));
		}
	}

}
