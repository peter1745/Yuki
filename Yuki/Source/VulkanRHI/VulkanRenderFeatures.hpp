#pragma once

#include "Features/VulkanCoreFeature.hpp"
#include "Features/VulkanRaytracingFeature.hpp"

namespace Yuki::Vulkan {

	inline Unique<VulkanFeature> GetVulkanFeature(RHI::RendererFeature feature)
	{
		using enum RHI::RendererFeature;
		switch (feature)
		{
		case RayTracing: return Unique<VulkanRaytracingFeature>::New();
		}

		return nullptr;
	}

}
