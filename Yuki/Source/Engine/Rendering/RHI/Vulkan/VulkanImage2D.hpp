#pragma once

#include "Rendering/RHI/Image2D.hpp"

#include "VulkanRenderContext.hpp"

namespace Yuki {

	class VulkanImage2D : public Image2D
	{
	public:
		uint32_t GetWidth() const override { return m_Width;}
		uint32_t GetHeight() const override { return m_Height; }
		ImageFormat GetImageFormat() const override { return m_Format; }

	private:
		static VulkanImage2D* Create(VulkanRenderContext* InContext, uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat);
		static void Destroy(VulkanRenderContext* InContext, VulkanImage2D* InImage);

	private:
		uint32_t m_Width;
		uint32_t m_Height;
		ImageFormat m_Format;

	private:
		friend class VulkanRenderContext;
	};

	class VulkanImageView2D : public ImageView2D
	{
	public:
		Image2D* GetImage() const override { return m_Image; }

	private:
		static VulkanImageView2D* Create(VulkanRenderContext* InContext, VulkanImage2D* InImage);
		static void Destroy(VulkanRenderContext* InContext, VulkanImageView2D* InImageView);

	private:
		VulkanImage2D* m_Image = nullptr;

	private:
		friend class VulkanRenderContext;
	};

}
