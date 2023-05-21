#pragma once

#include "Rendering/RenderContext.hpp"

#include "Vulkan.hpp"
#include "VulkanPlatform.hpp"
#include "VulkanDevice.hpp"
#include "VulkanShaderCompiler.hpp"

namespace Yuki {

	class VulkanRenderContext : public RenderContext
	{
	public:
		VulkanRenderContext();
		~VulkanRenderContext() = default;

		void Initialize() override;
		void Destroy() override;

		Unique<Swapchain> CreateSwapchain(GenericWindow* InWindow) const override;

		ShaderManager* GetShaderManager() const override { return m_ShaderManager.GetPtr(); }
		ShaderCompiler* GetShaderCompiler() const override { return m_ShaderCompiler.GetPtr(); }

	private:
		bool HasValidationLayerSupport() const;
		void SelectSuitablePhysicalDevice();

	private:
		VkInstance m_Instance = VK_NULL_HANDLE;
		VulkanDevice* m_Device = nullptr;
		VkQueue m_Queue = VK_NULL_HANDLE;

		Unique<ShaderManager> m_ShaderManager = nullptr;
		Unique<VulkanShaderCompiler> m_ShaderCompiler = nullptr;
	};

}
