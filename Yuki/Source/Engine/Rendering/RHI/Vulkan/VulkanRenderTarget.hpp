#pragma once

#include "Rendering/RHI/RenderTarget.hpp"

#include "Vulkan.hpp"

namespace Yuki {

	class VulkanRenderTarget : public RenderTarget
	{
	public:
		const RenderTargetInfo& GetInfo() const override { return m_Info; }

		const std::vector<AttachmentInfo>& GetColorAttachmentInfos() const override { return m_ColorAttachmentInfos; }
		const AttachmentInfo& GetDepthAttachmentInfo() const override { return m_DepthAttachmentInfo; }

		const std::vector<VkRenderingAttachmentInfo>& GetColorRenderingAttachments() const { return m_ColorAttachments; }
		const VkRenderingAttachmentInfo& GetDepthRenderingAttachment() const { return m_DepthAttachment; }

	private:
		static RenderTarget* Create(RenderContext* InContext, const RenderTargetInfo& InInfo);
		static void Destroy(RenderContext* InContext, VulkanRenderTarget* InRenderTarget);

	private:
		RenderTargetInfo m_Info;

		std::vector<AttachmentInfo> m_ColorAttachmentInfos;
		std::vector<VkRenderingAttachmentInfo> m_ColorAttachments;

		AttachmentInfo m_DepthAttachmentInfo;
		VkRenderingAttachmentInfo m_DepthAttachment;

	private:
		friend class VulkanRenderContext;
	};

}
