#include "VulkanBuffer.hpp"
#include "VulkanRenderContext.hpp"

namespace Yuki {

	Buffer VulkanRenderContext::CreateBuffer(const BufferInfo& InBufferInfo)
	{
		auto[handle, buffer] = m_Buffers.Acquire();
		buffer.Type = InBufferInfo.Type;
		buffer.Size = InBufferInfo.Size;

		VmaAllocationCreateInfo allocationInfo = { .usage = VMA_MEMORY_USAGE_AUTO };
		
		if (InBufferInfo.Type == BufferType::StagingBuffer)
		{
			allocationInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			allocationInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
		}

		buffer.UsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		switch (InBufferInfo.Type)
		{
		case BufferType::VertexBuffer:
		{
			buffer.UsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			break;
		}
		case BufferType::IndexBuffer:
		{
			buffer.UsageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			break;
		}
		case BufferType::StorageBuffer:
		{
			buffer.UsageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			break;
		}
		case BufferType::StagingBuffer:
		{
			buffer.UsageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			break;
		}
		}

		const auto& queue = m_Queues.Get(m_GraphicsQueue);

		VkBufferCreateInfo bufferCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.size = buffer.Size,
			.usage = buffer.UsageFlags,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &queue.FamilyIndex,
		};

		vmaCreateBuffer(m_Allocator, &bufferCreateInfo, &allocationInfo, &buffer.Handle, &buffer.Allocation, nullptr);

		return handle;
	}

	void VulkanRenderContext::Destroy(Buffer InBuffer)
	{
		auto& buffer = m_Buffers.Get(InBuffer);
		vmaDestroyBuffer(m_Allocator, buffer.Handle, buffer.Allocation);
		m_Buffers.Return(InBuffer);
	}

	void VulkanRenderContext::BufferSetData(Buffer InBuffer, const void* InData, uint32_t InDataSize)
	{
		auto& buffer = m_Buffers.Get(InBuffer);
		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(m_Allocator, buffer.Allocation, &allocationInfo);
		memcpy(allocationInfo.pMappedData, InData, size_t(InDataSize));
	}

}
