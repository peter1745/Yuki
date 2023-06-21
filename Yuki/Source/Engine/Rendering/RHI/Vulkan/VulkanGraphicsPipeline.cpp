#include "VulkanGraphicsPipeline.hpp"
#include "VulkanRenderContext.hpp"

namespace Yuki {

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
        vkDestroyPipelineLayout(m_Context->GetDevice(), m_Layout, nullptr);
        vkDestroyPipeline(m_Context->GetDevice(), m_Pipeline, nullptr);
    }

}
