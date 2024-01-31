#pragma once

#include "VulkanCommon.hpp"
#include "VulkanMemoryAllocator.hpp"
#include "ShaderCompiler.hpp"

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

		Aura::Unique<ShaderCompiler> Compiler;
	};

	template<>
	struct Handle<Swapchain>::Impl
	{
		RHIContext Context;
		Window Target;

		VkSwapchainKHR Resource;
		VkSurfaceKHR Surface;

		std::vector<Image> Images;
		std::vector<ImageView> ImageViews;
		uint32_t CurrentImageIndex;

		std::vector<VkSemaphore> Semaphores;
		uint32_t CurrentSemaphoreIndex;

		void Recreate();
	};

	inline VkImageLayout ImageLayoutToVkImageLayout(ImageLayout layout)
	{
		switch (layout)
		{
		case ImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
		case ImageLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
		case ImageLayout::AttachmentOptimal: return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		case ImageLayout::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		YukiAssert(false);
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	inline VkFormat ImageFormatToVkFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGBA8Unorm: return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::BGRA8Unorm: return VK_FORMAT_B8G8R8A8_UNORM;
		}

		YukiAssert(false);
		return VK_FORMAT_UNDEFINED;
	}

	inline VkImageUsageFlags ImageUsageToVkImageUsage(ImageUsage usage)
	{
		VkImageUsageFlags result = 0;

		if (usage & ImageUsage::ColorAttachment) result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (usage & ImageUsage::DepthStencilAttachment) result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (usage & ImageUsage::TransferSrc) result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (usage & ImageUsage::TransferDst) result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		return result;
	}

	template<>
	struct Handle<Image>::Impl
	{
		RHIContext Context;
		ImageAllocation Allocation;

		VkFormat Format;
		uint32_t Width;
		uint32_t Height;
		VkImageLayout OldLayout;
		VkImageLayout Layout;
		VkImageAspectFlags AspectFlags;
	};

	template<>
	struct Handle<ImageView>::Impl
	{
		RHIContext Context;
		Image Source;
		VkImageView Resource;
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

	inline VkShaderStageFlagBits ShaderStageToVkShaderStage(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		YukiAssert(false);
		return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}

	template<>
	struct Handle<GraphicsPipeline>::Impl
	{
		RHIContext Context;
		VkPipelineLayout Layout;
		VkPipeline Resource;
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
