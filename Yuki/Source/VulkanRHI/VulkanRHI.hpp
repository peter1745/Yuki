#pragma once

#include "Engine/Common/UniqueID.hpp"
#include "Engine/RHI/RenderHandles.hpp"

#include "Features/VulkanFeature.hpp"

#include <vma/vk_mem_alloc.h>

namespace Yuki::RHI {

	class VulkanShaderCompiler;

	template<>
	struct RenderHandle<Context>::Impl
	{
		VkInstance Instance = VK_NULL_HANDLE;
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkDevice Device = VK_NULL_HANDLE;

		VmaAllocator Allocator = VK_NULL_HANDLE;

		DynamicArray<QueueRH> Queues;

		bool ValidationSupport = false;
		VkDebugUtilsMessengerEXT DebugMessengerHandle = VK_NULL_HANDLE;

		HashMap<RendererFeature, Unique<VulkanFeature>> EnabledFeatures;

		Unique<VulkanShaderCompiler> ShaderCompiler;

		template<typename TFeatureClass>
		TFeatureClass& GetFeature() const
		{
			return EnabledFeatures.at(TFeatureClass::GetRendererFeature());
		}
	};

	template<>
	struct RenderHandle<Queue>::Impl
	{
		Context Ctx = {};
		VkQueue Handle = VK_NULL_HANDLE;
		uint32_t Family = std::numeric_limits<uint32_t>::max();
		uint32_t Index = 0;
		VkQueueFlags Flags = 0;
	};

	template<>
	struct RenderHandle<Swapchain>::Impl
	{
		Context Ctx = {};

		VkSwapchainKHR Handle = VK_NULL_HANDLE;
		VkSurfaceKHR Surface = VK_NULL_HANDLE;
		VkSurfaceFormatKHR SurfaceFormat = {};
		DynamicArray<Image> Images;
		DynamicArray<ImageView> ImageViews;
		uint32_t CurrentImageIndex = 0;
		DynamicArray<VkSemaphore> Semaphores;
		uint32_t CurrentSemaphoreIndex = 0;

		const WindowSystem* WindowingSystem = nullptr;
		UniqueID TargetWindow;
	};

	template<>
	struct RenderHandle<Fence>::Impl
	{
		Context Ctx = {};
		VkSemaphore Handle = VK_NULL_HANDLE;
		uint64_t Value = 0;
	};

	template<>
	struct RenderHandle<Image>::Impl
	{
		VkImage Handle = VK_NULL_HANDLE;
		uint32_t Width = 0;
		uint32_t Height = 0;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkImageAspectFlags AspectMask = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		inline static VkImageLayout ImageLayoutToVkImageLayout(ImageLayout InLayout)
		{
			switch (InLayout)
			{
			case ImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
			case ImageLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
			case ImageLayout::Attachment: return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			case ImageLayout::ShaderReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case ImageLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			case ImageLayout::TransferDest: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case ImageLayout::TransferSource: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			default: return VK_IMAGE_LAYOUT_UNDEFINED;
			}
		}
	};

	template<>
	struct RenderHandle<ImageView>::Impl
	{
		Context Ctx = {};
		VkImageView Handle = VK_NULL_HANDLE;
		ImageRH Image = {};
	};

	template<>
	struct RenderHandle<DescriptorSetLayout>::Impl
	{
		VkDescriptorSetLayout Handle;
	};

	template<>
	struct RenderHandle<DescriptorPool>::Impl
	{
		Context Ctx = {};
		VkDescriptorPool Handle;
		DynamicArray<DescriptorSet> AllocatedSets;
	};

	template<>
	struct RenderHandle<DescriptorSet>::Impl
	{
		Context Ctx = {};
		VkDescriptorSet Handle;
		DescriptorSetLayoutRH Layout = {};
	};

	template<>
	struct RenderHandle<CommandPool>::Impl
	{
		Context Ctx = {};

		VkCommandPool Handle = VK_NULL_HANDLE;
		DynamicArray<CommandList> AllocatedLists;
		size_t NextList = 0;
	};

	template<>
	struct RenderHandle<CommandList>::Impl
	{
		VkCommandBuffer Handle = VK_NULL_HANDLE;
	};

	template<>
	struct RenderHandle<Buffer>::Impl
	{
		Context Ctx = {};
		VkBuffer Handle = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VkDeviceAddress Address = 0;
		uint64_t Size = 0;
	};

	template<>
	struct RenderHandle<Pipeline>::Impl
	{
		VkPipeline Handle;
		VkPipelineLayout Layout;
	};

	template<>
	struct RenderHandle<RayTracingPipeline>::Impl
	{
		VkPipeline Handle;
		VkPipelineLayout Layout;

		Buffer SBTBuffer = {};
		VkStridedDeviceAddressRegionKHR RayGenRegion{};
		VkStridedDeviceAddressRegionKHR MissGenRegion{};
		VkStridedDeviceAddressRegionKHR ClosestHitGenRegion{};
		VkStridedDeviceAddressRegionKHR CallableGenRegion{};
	};

	template<>
	struct RenderHandle<AccelerationStructure>::Impl
	{
		Context Ctx = {};
		VkAccelerationStructureKHR BottomLevelAS;
		Buffer AccelerationStructureStorage;
		VkAccelerationStructureKHR TopLevelAS;
		Buffer TopLevelAccelerationStructureStorage;
	};

}
