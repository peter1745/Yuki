#pragma once

#include "VulkanCommon.hpp"
#include "VulkanMemoryAllocator.hpp"

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
	struct Handle<Queue>::Impl
	{
		RHIContext Context;
		VkQueue Queue;
		uint32_t Family;
		uint32_t Index;
		VkQueueFlags Flags;
		float Priority;
	};

}
