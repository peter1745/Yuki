#pragma once

#include "Rendering/RHI/DescriptorSet.hpp"

#include "VulkanImage2D.hpp"
#include "VulkanSampler.hpp"

namespace Yuki {

	struct VulkanDescriptorSetLayout : public DescriptorSetLayout
	{
		VkDescriptorSetLayout Handle;
	};

	class VulkanRenderContext;

	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		void Write(uint32_t InBinding, std::span<Image2D* const> InImages, Sampler* InSampler) override;
		void Write(uint32_t InBinding, Buffer* InBuffer) override;

		DescriptorSetLayout* GetLayout() const override { return m_Layout.GetPtr(); }

	private:
		VulkanRenderContext* m_Context = nullptr;
		Unique<VulkanDescriptorSetLayout> m_Layout = nullptr;
		VkDescriptorSet m_Set = VK_NULL_HANDLE;

		friend class VulkanDescriptorPool;
		friend class VulkanCommandBuffer;
	};

	class VulkanDescriptorPool : public DescriptorPool
	{
	public:
		DescriptorSet* AllocateSet(DescriptorSetLayout* InSetLayout) override;

	private:
		VulkanDescriptorPool(VulkanRenderContext* InContext, std::span<DescriptorCount> InDescriptorCounts);

	private:
		VulkanRenderContext* m_Context = nullptr;
		VkDescriptorPool m_Pool = VK_NULL_HANDLE;

		List<VulkanDescriptorSet*> m_Sets;

		friend class VulkanRenderContext;
	};

}
