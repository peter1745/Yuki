#pragma once

#include "Rendering/RHI/GraphicsPipeline.hpp"

#include "VulkanShader.hpp"

namespace Yuki {

	class VulkanRenderContext;
	class VulkanShader;

	class VulkanGraphicsPipeline : public GraphicsPipeline
	{
	public:
		~VulkanGraphicsPipeline();

		VkPipeline GetVkPipeline() const { return m_Pipeline; }
		VkPipelineLayout GetVkPipelineLayout() const { return m_Layout; }

		Shader* GetShader() override { return m_Shader; }

	private:
		VulkanRenderContext* m_Context = nullptr;
		VulkanShader* m_Shader = nullptr;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_Layout = VK_NULL_HANDLE;

		friend class VulkanGraphicsPipelineBuilder;
	};

}
