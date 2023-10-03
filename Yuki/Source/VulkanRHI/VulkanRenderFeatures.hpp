#pragma once

#include "Features/VulkanRaytracingFeature.hpp"

namespace Yuki::Vulkan {

	inline Unique<VulkanFeature> GetVulkanFeature(RHI::RendererFeature InFeature)
	{
		using enum RHI::RendererFeature;
		switch (InFeature)
		{
		case RayTracing: return Unique<VulkanRaytracingFeature>::New();
		}

		return nullptr;
	}

}
