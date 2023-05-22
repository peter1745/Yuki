#include "VulkanImage2D.hpp"

namespace Yuki {

	VulkanImage2D* VulkanImage2D::Create(VulkanRenderContext* InContext, uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat)
	{
		return nullptr;
	}

	void VulkanImage2D::Destroy(VulkanRenderContext* InContext, VulkanImage2D* InImage)
	{
	}

	VulkanImageView2D* VulkanImageView2D::Create(VulkanRenderContext* InContext, VulkanImage2D* InImage)
	{
		return nullptr;
	}

	void VulkanImageView2D::Destroy(VulkanRenderContext* InContext, VulkanImageView2D* InImageView)
	{
	}

}
