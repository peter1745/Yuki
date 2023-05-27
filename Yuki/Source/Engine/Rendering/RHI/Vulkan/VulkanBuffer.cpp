#include "VulkanBuffer.hpp"
#include "VulkanHelper.hpp"

namespace Yuki {

	VulkanBuffer::VulkanBuffer(VulkanRenderContext* InContext, const BufferInfo& InInfo)
		: m_Context(InContext), m_Info(InInfo)
	{
		uint32_t queueFamilyIndex = static_cast<VulkanQueue*>(InContext->GetGraphicsQueue())->GetFamilyIndex();
		VkBufferCreateInfo bufferInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = InInfo.Size,
			.usage = VulkanHelper::BufferTypeToVkUsageFlags(InInfo.Type),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &queueFamilyIndex
		};

		m_Buffer = InContext->GetAllocator().CreateBuffer(InInfo.Type, &bufferInfo, &m_Allocation);

		if (InInfo.PersistentlyMapped)
			m_MappedData = m_Context->GetAllocator().MapMemory(m_Allocation);
	}

	VulkanBuffer::~VulkanBuffer()
	{
		if (m_MappedData)
		{
			m_Context->GetAllocator().UnmapMemory(m_Allocation);
			m_MappedData = nullptr;
		}

		m_Context->GetAllocator().DestroyBuffer(m_Buffer, m_Allocation);
	}

	void VulkanBuffer::SetData(void* InData, uint32_t InDataSize, uint32_t InDstOffset /*= 0*/)
	{
		YUKI_VERIFY(InDataSize + InDstOffset < m_Info.Size);

		if (!m_Info.PersistentlyMapped)
			m_MappedData = m_Context->GetAllocator().MapMemory(m_Allocation);

		memcpy(reinterpret_cast<std::byte*>(m_MappedData) + InDstOffset, InData, InDataSize);

		if (!m_Info.PersistentlyMapped)
		{
			m_Context->GetAllocator().UnmapMemory(m_Allocation);
			m_MappedData = nullptr;
		}
	}

	void VulkanBuffer::UploadData(Buffer* InStagingBuffer, const BufferUploadInfo* InUploadInfo)
	{
		uint32_t srcOffset = 0;
		uint32_t dstOffset = 0;
		uint32_t size = m_Info.Size;

		if (InUploadInfo != nullptr)
		{
			srcOffset = InUploadInfo->SrcOffset;
			dstOffset = InUploadInfo->DstOffset;
			size = InUploadInfo->Size;
		}

		VkCommandBuffer commandBuffer = m_Context->CreateTransientCommandBuffer();

		VkBufferCopy2 copyRegion =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
			.srcOffset = srcOffset,
			.dstOffset = dstOffset,
			.size = size,
		};

		VkCopyBufferInfo2 bufferCopyInfo = {
			.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
			.srcBuffer = static_cast<VulkanBuffer*>(InStagingBuffer)->m_Buffer,
			.dstBuffer = m_Buffer,
			.regionCount = 1,
			.pRegions = &copyRegion
		};

		vkCmdCopyBuffer2(commandBuffer, &bufferCopyInfo);

		vkEndCommandBuffer(commandBuffer);

		static_cast<VulkanQueue*>(m_Context->GetGraphicsQueue())->SubmitCommandBuffers(std::span<VkCommandBuffer>(&commandBuffer, 1), {}, {});
	}

}