#pragma once

#include "Rendering/RHI/Shader.hpp"

#include "VulkanInclude.hpp"

namespace Yuki {

    class VulkanRenderContext;

    class VulkanShader : public Shader
    {
    public:
        ~VulkanShader();

        std::string_view GetName() const override { return m_Name; }

    private:
        VulkanRenderContext* m_Context = nullptr;
        std::string m_Name;
		Map<ShaderModuleType, VkShaderModule> m_ModuleHandles;

        friend class VulkanGraphicsPipelineBuilder;
        friend class VulkanShaderCompiler;
    };

}
