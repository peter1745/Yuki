#pragma once

#include "Rendering/RHI/Image2D.hpp"

#include "VulkanRenderContext.hpp"

namespace Yuki {

	struct VulkanImageTransition
	{
		VkPipelineStageFlags2 DstPipelineStage;
		VkAccessFlags2 DstAccessFlags;
		VkImageLayout DstImageLayout;
	};

	class VulkanImage2D : public Image2D
	{
	public:
		~VulkanImage2D();

		uint32_t GetWidth() const override { return m_Width;}
		uint32_t GetHeight() const override { return m_Height; }
		ImageFormat GetImageFormat() const override { return m_Format; }

		VkImage GetVkImage() const { return m_Image; }

		void Transition(VkCommandBuffer InCommandBuffer, const VulkanImageTransition& InTransitionInfo);

	private:
		VulkanImage2D(VulkanRenderContext* InContext, uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat);
		VulkanImage2D(VulkanRenderContext* InContext, uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, VkImage InExistingImage);

	private:
		VulkanRenderContext* m_Context = nullptr;
		uint32_t m_Width;
		uint32_t m_Height;
		ImageFormat m_Format;

		VkImage m_Image;
		VmaAllocation m_Allocation;

		VkPipelineStageFlags2 m_CurrentPipelineStage = VK_PIPELINE_STAGE_2_NONE;
		VkAccessFlags2 m_CurrentAccessFlags = VK_ACCESS_2_NONE;
		VkImageLayout m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	private:
		friend class VulkanRenderContext;
		friend class VulkanSwapchain;
	};

	class VulkanImageView2D : public ImageView2D
	{
	public:
		~VulkanImageView2D();

		Image2D* GetImage() const override { return m_Image; }

		VkImageView GetVkImageView() const { return m_ImageView; }

	private:
		VulkanImageView2D(VulkanRenderContext* InContext, VulkanImage2D* InImage);

	private:
		VulkanRenderContext* m_Context = nullptr;
		VulkanImage2D* m_Image = nullptr;
		VkImageView m_ImageView = VK_NULL_HANDLE;

	private:
		friend class VulkanRenderContext;
	};

}