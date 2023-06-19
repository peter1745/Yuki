#pragma once

#include "Rendering/RHI/Buffer.hpp"

#include "VulkanRenderContext.hpp"

namespace Yuki {

	class VulkanBuffer : public Buffer
	{
	public:
		~VulkanBuffer();
		
		void SetData(void* InData, uint32_t InDataSize, uint32_t InDstOffset = 0) override;

		void UploadData(Buffer* InStagingBuffer, const BufferUploadInfo* InUploadInfo) override;

		const BufferInfo& GetInfo() const override { return m_Info; }

		VkBuffer GetVkBuffer() const { return m_Buffer; }

	private:
		VulkanBuffer(VulkanRenderContext* InContext, const BufferInfo& InInfo);

	private:
		VulkanRenderContext* m_Context = nullptr;
		BufferInfo m_Info;
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation;

		void* m_MappedData = nullptr;

	private:
		friend class VulkanRenderContext;
	};

}
