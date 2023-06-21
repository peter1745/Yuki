#pragma once

#include <span>

namespace Yuki {

	class Image2D;
	class Sampler;

	enum class DescriptorType
	{
		UniformBuffer,
		CombinedImageSampler
	};

	struct DescriptorSetLayout {};

	class DescriptorSet
	{
	public:
		virtual ~DescriptorSet() = default;

		virtual void Write(uint32_t InBinding, std::span<Image2D* const> InImages, Sampler* InSampler) = 0;

		virtual DescriptorSetLayout* GetLayout() const = 0;
	};

	struct DescriptorCount
	{
		DescriptorType Type;
		uint32_t Count;
	};

	class DescriptorPool
	{
	public:
		virtual ~DescriptorPool() = default;

		virtual DescriptorSet* AllocateSet(DescriptorSetLayout* InSetLayout) = 0;
	};

}
