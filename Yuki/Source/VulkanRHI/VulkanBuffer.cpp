#include "VulkanRHI.hpp"

namespace Yuki::RHI {

	static VkBufferUsageFlags BufferUsageToVkBufferUsageFlags(BufferUsage usage)
	{
		VkBufferUsageFlags result = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		if (usage & BufferUsage::Vertex) result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (usage & BufferUsage::Index) result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (usage & BufferUsage::Storage) result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (usage & BufferUsage::TransferSrc) result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (usage & BufferUsage::TransferDst) result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if (usage & BufferUsage::ShaderBindingTable) result |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
		if (usage & BufferUsage::AccelerationStructureStorage) result |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
		if (usage & BufferUsage::AccelerationStructureBuildInput) result |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

		return result;
	}

	Buffer Buffer::Create(Context context, uint64_t size, BufferUsage usage, bool hostAccess)
	{
		auto buffer = new Impl();
		buffer->Ctx = context;
		buffer->Size = size;

		VmaAllocationCreateInfo allocationInfo = { .usage = VMA_MEMORY_USAGE_AUTO, };

		if (hostAccess)
		{
			allocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		}

		VkBufferCreateInfo bufferInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.size = size,
			.usage = BufferUsageToVkBufferUsageFlags(usage),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
		};

		vmaCreateBuffer(context->Allocator, &bufferInfo, &allocationInfo, &buffer->Handle, &buffer->Allocation, nullptr);

		VkBufferDeviceAddressInfo addressInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = buffer->Handle
		};
		buffer->Address = vkGetBufferDeviceAddress(context->Device, &addressInfo);

		return { buffer };
	}

	void Buffer::SetData(const void* data, uint64_t dataSize)
	{
		dataSize = std::min(dataSize, m_Impl->Size);

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(m_Impl->Ctx->Allocator, m_Impl->Allocation, &allocationInfo);
		memcpy(allocationInfo.pMappedData, data, dataSize);
	}

	uint64_t Buffer::GetDeviceAddress()
	{
		return m_Impl->Address;
	}

	void* Buffer::GetMappedMemory()
	{
		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(m_Impl->Ctx->Allocator, m_Impl->Allocation, &allocationInfo);
		return allocationInfo.pMappedData;
	}

	void Buffer::Destroy()
	{
		vmaDestroyBuffer(m_Impl->Ctx->Allocator, m_Impl->Handle, m_Impl->Allocation);
		delete m_Impl;
	}

}
