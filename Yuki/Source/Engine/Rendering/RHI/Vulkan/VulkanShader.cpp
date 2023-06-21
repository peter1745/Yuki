#include "VulkanShader.hpp"
#include "VulkanRenderContext.hpp"

namespace Yuki {

    VulkanShader::~VulkanShader()
    {
        for (auto[moduleType, moduleHandle] : m_ModuleHandles)
            vkDestroyShaderModule(m_Context->GetDevice(), moduleHandle, nullptr);
    }

}
