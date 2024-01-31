#pragma once

#include <Engine/Core/Core.hpp>

#include <volk.h>

#define NOMINMAX
#include <vulkan/vulkan.h>

#include <string_view>

namespace Yuki::Vulkan {

	inline std::string_view VkResultToString(VkResult result)
	{
		if (result == VK_SUCCESS) return "VK_SUCCESS";
		if (result == VK_NOT_READY) return "VK_NOT_READY";
		if (result == VK_TIMEOUT) return "VK_TIMEOUT";
		if (result == VK_EVENT_SET) return "VK_EVENT_SET";
		if (result == VK_EVENT_RESET) return "VK_EVENT_RESET";
		if (result == VK_INCOMPLETE) return "VK_INCOMPLETE";
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) return "VK_ERROR_OUT_OF_HOST_MEMORY";
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		if (result == VK_ERROR_INITIALIZATION_FAILED) return "VK_ERROR_INITIALIZATION_FAILED";
		if (result == VK_ERROR_DEVICE_LOST) return "VK_ERROR_DEVICE_LOST";
		if (result == VK_ERROR_MEMORY_MAP_FAILED) return "VK_ERROR_MEMORY_MAP_FAILED";
		if (result == VK_ERROR_LAYER_NOT_PRESENT) return "VK_ERROR_LAYER_NOT_PRESENT";
		if (result == VK_ERROR_EXTENSION_NOT_PRESENT) return "VK_ERROR_EXTENSION_NOT_PRESENT";
		if (result == VK_ERROR_FEATURE_NOT_PRESENT) return "VK_ERROR_FEATURE_NOT_PRESENT";
		if (result == VK_ERROR_INCOMPATIBLE_DRIVER) return "VK_ERROR_INCOMPATIBLE_DRIVER";
		if (result == VK_ERROR_TOO_MANY_OBJECTS) return "VK_ERROR_TOO_MANY_OBJECTS";
		if (result == VK_ERROR_FORMAT_NOT_SUPPORTED) return "VK_ERROR_FORMAT_NOT_SUPPORTED";
		if (result == VK_ERROR_FRAGMENTED_POOL) return "VK_ERROR_FRAGMENTED_POOL";
		if (result == VK_ERROR_UNKNOWN) return "VK_ERROR_UNKNOWN";
		if (result == VK_ERROR_OUT_OF_POOL_MEMORY) return "VK_ERROR_OUT_OF_POOL_MEMORY";
		if (result == VK_ERROR_INVALID_EXTERNAL_HANDLE) return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
		if (result == VK_ERROR_FRAGMENTATION) return "VK_ERROR_FRAGMENTATION";
		if (result == VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS) return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
		if (result == VK_PIPELINE_COMPILE_REQUIRED) return "VK_PIPELINE_COMPILE_REQUIRED";
		if (result == VK_ERROR_SURFACE_LOST_KHR) return "VK_ERROR_SURFACE_LOST_KHR";
		if (result == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		if (result == VK_SUBOPTIMAL_KHR) return "VK_SUBOPTIMAL_KHR";
		if (result == VK_ERROR_OUT_OF_DATE_KHR) return "VK_ERROR_OUT_OF_DATE_KHR";
		if (result == VK_ERROR_INCOMPATIBLE_DISPLAY_KHR) return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		if (result == VK_ERROR_VALIDATION_FAILED_EXT) return "VK_ERROR_VALIDATION_FAILED_EXT";
		if (result == VK_ERROR_INVALID_SHADER_NV) return "VK_ERROR_INVALID_SHADER_NV";
		if (result == VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR) return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
		if (result == VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR) return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
		if (result == VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR) return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
		if (result == VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR) return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
		if (result == VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR) return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
		if (result == VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR) return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
		if (result == VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT) return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
		if (result == VK_ERROR_NOT_PERMITTED_KHR) return "VK_ERROR_NOT_PERMITTED_KHR";
		if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT) return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
		if (result == VK_THREAD_IDLE_KHR) return "VK_THREAD_IDLE_KHR";
		if (result == VK_THREAD_DONE_KHR) return "VK_THREAD_DONE_KHR";
		if (result == VK_OPERATION_DEFERRED_KHR) return "VK_OPERATION_DEFERRED_KHR";
		if (result == VK_OPERATION_NOT_DEFERRED_KHR) return "VK_OPERATION_NOT_DEFERRED_KHR";
		if (result == VK_ERROR_COMPRESSION_EXHAUSTED_EXT) return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
		if (result == VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT) return "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT";
		if (result == VK_ERROR_OUT_OF_POOL_MEMORY_KHR) return "VK_ERROR_OUT_OF_POOL_MEMORY_KHR";
		if (result == VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR) return "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR";
		if (result == VK_ERROR_FRAGMENTATION_EXT) return "VK_ERROR_FRAGMENTATION_EXT";
		if (result == VK_ERROR_NOT_PERMITTED_EXT) return "VK_ERROR_NOT_PERMITTED_EXT";
		if (result == VK_ERROR_INVALID_DEVICE_ADDRESS_EXT) return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
		if (result == VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR) return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR";
		if (result == VK_PIPELINE_COMPILE_REQUIRED_EXT) return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
		if (result == VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT) return "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT";
		if (result == VK_RESULT_MAX_ENUM) return "VK_RESULT_MAX_ENUM";

		return "Unknown";
	}

	inline void CheckResult(VkResult result)
	{
		if (result != VK_SUCCESS)
		{
			::Yuki::WriteLine("VkResult was {}", VkResultToString(result));
			YukiAssert(false);
		}
	}

	template<typename EnumerateFunc, typename VulkanType, typename... Args>
	requires SameReturn<EnumerateFunc, VkResult, Args..., uint32_t*, VulkanType*>
	void Enumerate(EnumerateFunc&& enumerate, std::vector<VulkanType>& result, Args&&... args)
	{
		uint32_t elementCount = 0;
		CheckResult(enumerate(std::forward<Args>(args)..., &elementCount, nullptr));
		result.resize(static_cast<size_t>(elementCount));
		CheckResult(enumerate(std::forward<Args>(args)..., &elementCount, result.data()));
	}

	template<typename EnumerateFunc, typename VulkanType, typename... Args>
	requires SameReturn<EnumerateFunc, void, Args..., uint32_t*, VulkanType*>
	void Enumerate(EnumerateFunc&& enumerate, std::vector<VulkanType>& result, Args&&... args)
	{
		uint32_t elementCount = 0;
		enumerate(std::forward<Args>(args)..., &elementCount, nullptr);
		result.resize(static_cast<size_t>(elementCount));
		enumerate(std::forward<Args>(args)..., &elementCount, result.data());
	}

}
