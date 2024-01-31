#pragma once

#include "VulkanCommon.hpp"
#include "VulkanMemoryAllocator.hpp"

#include <Engine/Core/Window.hpp>
#include <Engine/RHI/RHI.hpp>

namespace Yuki {

	template<>
	struct Handle<RHIContext>::Impl
	{
		VkInstance Instance;
		bool ValidationEnabled;
		VkDebugUtilsMessengerEXT DebugMessenger;
		VkPhysicalDevice PhysicalDevice;
		VkDevice Device;

		std::vector<Queue> Queues;

		VulkanMemoryAllocator Allocator;
	};

	template<>
	struct Handle<Fence>::Impl
	{
		RHIContext Context;
		VkSemaphore Resource;
		uint64_t Value;
	};

	template<>
	struct Handle<Queue>::Impl
	{
		RHIContext Context;
		VkQueue Queue;
		uint32_t Family;
		uint32_t Index;
		VkQueueFlags Flags;
		float Priority;
	};

	template<>
	struct Handle<Swapchain>::Impl
	{
		RHIContext Context;
		Window Target;

		VkSwapchainKHR Resource;
		VkSurfaceKHR Surface;

		std::vector<VkImage> Images;
		uint32_t CurrentImageIndex;

		std::vector<VkSemaphore> Semaphores;
		uint32_t CurrentSemaphoreIndex;

		void Recreate();
	};

	template<>
	struct Handle<CommandList>::Impl
	{
		VkCommandBuffer Resource;
	};

	template<>
	struct Handle<CommandPool>::Impl
	{
		RHIContext Context;
		VkCommandPool Resource;

		std::vector<CommandList> AllocatedLists;
		uint32_t NextList = 0;
	};

}
