#pragma once

#include "Engine/Common/Core.hpp"
#include "Engine/RHI/RenderFeatures.hpp"

#include "../VulkanInclude.hpp"

#include <string_view>

namespace Yuki {

	template<typename TExistingFeatures, typename TNewFeatures>
	void AddToPNext(TExistingFeatures& InFeatures, TNewFeatures& InNewFeatures)
	{
		InNewFeatures.pNext = InFeatures.pNext;
		InFeatures.pNext = &InNewFeatures;
	}

#define YUKI_VULKAN_FEATURE_IMPL(FeatureType) static RHI::RendererFeature GetRendererFeature() { return RHI::RendererFeature::FeatureType; }

	class VulkanFeature
	{
	public:
		virtual ~VulkanFeature() = default;

		RHI::RendererFeature GetRendererFeature() const { return m_Feature; }

		virtual const DynamicArray<std::string_view> GetRequiredExtensions() const = 0;

		virtual void PopulatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2& InDeviceFeatures) = 0;

		virtual void PopulateProperties(VkPhysicalDeviceProperties2& InProperties) {}

	protected:
		VulkanFeature(RHI::RendererFeature InFeature)
			: m_Feature(InFeature) {}

	private:
		RHI::RendererFeature m_Feature;
	};

}
