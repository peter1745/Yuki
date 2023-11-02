#pragma once

#include "Engine/Common/UniqueID.hpp"
#include "Engine/RHI/RenderHandles.hpp"

#include "Features/VulkanFeature.hpp"

#include "VulkanUtils.hpp"

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

		VkDescriptorSetLayout DescriptorHeapLayout = VK_NULL_HANDLE;

		CommandPool TemporariesPool;

		template<typename TFeatureClass>
		TFeatureClass& GetFeature() const
		{
			return EnabledFeatures.at(TFeatureClass::GetRendererFeature());
		}

		CommandList GetTemporaryCommandList();
		void EndTemporaryCommandList(CommandList cmd);
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
		uint64_t NextSignalValue = 0;
	};

	template<>
	struct RenderHandle<Image>::Impl
	{
		Context Ctx = {};

		VkImage Handle = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		uint32_t Width = 0;
		uint32_t Height = 0;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkImageAspectFlags AspectMask = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		ImageView DefaultView{};

		inline static VkImageLayout ImageLayoutToVkImageLayout(ImageLayout layout)
		{
			switch (layout)
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

		inline static VkFormat ImageFormatToVkFormat(RHI::ImageFormat format)
		{
			switch (format)
			{
			case Yuki::RHI::ImageFormat::RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;
			case Yuki::RHI::ImageFormat::BGRA8: return VK_FORMAT_B8G8R8A8_UNORM;
			case Yuki::RHI::ImageFormat::D32SFloat: return VK_FORMAT_D32_SFLOAT;
			}

			return VK_FORMAT_UNDEFINED;
		}

		inline static VkImageUsageFlags ImageUsageToVkImageUsageFlags(ImageUsage usage)
		{
			VkImageUsageFlags result = 0;

			if (usage & ImageUsage::ColorAttachment)	result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			if (usage & ImageUsage::DepthAttachment)	result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			if (usage & ImageUsage::Sampled)			result |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if (usage & ImageUsage::TransferDest)		result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if (usage & ImageUsage::TransferSource)		result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if (usage & ImageUsage::Storage)			result |= VK_IMAGE_USAGE_STORAGE_BIT;
			if (usage & ImageUsage::HostTransfer)		result |= VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT;

			return result;
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
	struct RenderHandle<Sampler>::Impl
	{
		VkSampler Handle = VK_NULL_HANDLE;
	};

	template<>
	struct RenderHandle<DescriptorHeap>::Impl
	{
		Context Ctx = {};
		uint32_t NumDescriptors;
		VkDescriptorPool Handle;
		VkDescriptorSet Set;
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
		BufferUsage Usage;
		BufferFlags Flags;

		void CreateInternal(Context context, uint64_t size, BufferUsage usage, BufferFlags flags);
	};

	template<>
	struct RenderHandle<PipelineLayout>::Impl
	{
		VkPipelineLayout Handle;
	};

	template<>
	struct RenderHandle<Pipeline>::Impl
	{
		PipelineLayout Layout;
		VkPipeline Handle;
	};

	template<>
	struct RenderHandle<RayTracingPipeline>::Impl
	{
		PipelineLayout Layout;
		VkPipeline Handle;

		uint32_t HandleSize;
		uint32_t HandleStride;
		DynamicArray<uint8_t> Handles;

		Buffer SBTBuffer = {};
		VkStridedDeviceAddressRegionKHR RayGenRegion{};
		VkStridedDeviceAddressRegionKHR RayMissRegion{};
		VkStridedDeviceAddressRegionKHR RayHitRegion{};
		VkStridedDeviceAddressRegionKHR CallablesRegion{};
	};

	template<>
	struct RenderHandle<AccelerationStructure>::Impl
	{
		struct BLAS
		{
			Buffer Storage;
			VkAccelerationStructureKHR Structure;
		};

		Context Ctx = {};

		DynamicArray<BLAS> BottomLevelStructures;

		Buffer InstancesBuffer;
		uint32_t InstanceCount;
		VkAccelerationStructureKHR TopLevelStructure;
		Buffer TopLevelStructureStorage;
	};

	template<>
	struct RenderHandle<AccelerationStructureBuilder>::Impl
	{
		Context Ctx = {};
		VkQueryPool QueryPool = VK_NULL_HANDLE;

		struct GeometryIndexData
		{
			Buffer IndexBuffer;
			uint32_t IndexCount;
		};

		struct BLAS
		{
			DynamicArray<VkAccelerationStructureGeometryKHR> Geometries;
			DynamicArray<Buffer> VertexBuffers;
			DynamicArray<GeometryIndexData> IndexBuffers;
			VkAccelerationStructureBuildSizesInfoKHR BuildSizesInfo;
		};
		DynamicArray<BLAS> BottomLevelStructures;

		struct InstanceData
		{
			BlasID BLAS;
			GeometryID Geometry;
			Mat4 Transform;
			uint32_t CustomInstanceIndex;
			uint32_t SBTOffset;
		};
		DynamicArray<InstanceData> Instances;

		void BuildBottomLevelStructures(AccelerationStructure::Impl* accelerationStructure);
		void BuildTopLevelStructure(AccelerationStructure::Impl* accelerationStructure);
	};

}
