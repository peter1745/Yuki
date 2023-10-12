#pragma once

#include "Engine/Common/Core.hpp"
#include "Engine/RHI/RenderFeatures.hpp"

#include "../VulkanInclude.hpp"

#include <string_view>

namespace Yuki {

	template<typename TExistingFeatures, typename TNewFeatures>
	void AddToPNext(TExistingFeatures& features, TNewFeatures& newFeatures)
	{
		newFeatures.pNext = features.pNext;
		features.pNext = &newFeatures;
	}

#define YUKI_VULKAN_FEATURE_IMPL(FeatureType) static RHI::RendererFeature GetRendererFeature() { return RHI::RendererFeature::FeatureType; }

	class VulkanFeature
	{
	public:
		virtual ~VulkanFeature() = default;

		RHI::RendererFeature GetRendererFeature() const { return m_Feature; }

		virtual const DynamicArray<std::string_view> GetRequiredExtensions() const = 0;

		virtual void PopulatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures2& deviceFeatures) = 0;

		virtual void PopulateProperties(VkPhysicalDeviceProperties2& properties) {}

	protected:
		VulkanFeature(RHI::RendererFeature feature)
			: m_Feature(feature) {}

	private:
		RHI::RendererFeature m_Feature;
	};

}
