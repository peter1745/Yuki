#pragma once

#include "Features/VulkanCoreFeature.hpp"
#include "Features/VulkanRaytracingFeature.hpp"
#include "Features/VulkanHostImageCopyFeature.hpp"

namespace Yuki::Vulkan {

	inline Unique<VulkanFeature> GetVulkanFeature(RHI::RendererFeature feature)
	{
		using enum RHI::RendererFeature;
		switch (feature)
		{
		case RayTracing: return Unique<VulkanRaytracingFeature>::New();
		case HostImageCopy: return Unique<VulkanHostImageCopyFeature>::New();
		}

		return nullptr;
	}

}
