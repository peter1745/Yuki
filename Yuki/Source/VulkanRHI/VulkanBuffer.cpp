#include "VulkanRHI.hpp"

namespace Yuki::RHI {

	static VkBufferUsageFlags BufferUsageToVkBufferUsageFlags(BufferUsage InUsage)
	{
		VkBufferUsageFlags Result = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		if (InUsage & BufferUsage::Vertex) Result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (InUsage & BufferUsage::Index) Result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (InUsage & BufferUsage::Storage) Result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (InUsage & BufferUsage::TransferSrc) Result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (InUsage & BufferUsage::TransferDst) Result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if (InUsage & BufferUsage::ShaderBindingTable) Result |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
		if (InUsage & BufferUsage::AccelerationStructureStorage) Result |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
		if (InUsage & BufferUsage::AccelerationStructureBuildInput) Result |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

		return Result;
	}

	Buffer Buffer::Create(Context InContext, uint64_t InSize, BufferUsage InUsage, bool InHostAccess)
	{
		auto Buffer = new Impl();
		Buffer->Ctx = InContext;
		Buffer->Size = InSize;

		VmaAllocationCreateInfo AllocationInfo = { .usage = VMA_MEMORY_USAGE_AUTO, };

		if (InHostAccess)
		{
			AllocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		}

		VkBufferCreateInfo BufferInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.size = InSize,
			.usage = BufferUsageToVkBufferUsageFlags(InUsage),
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
		};

		vmaCreateBuffer(InContext->Allocator, &BufferInfo, &AllocationInfo, &Buffer->Handle, &Buffer->Allocation, nullptr);

		VkBufferDeviceAddressInfo AddressInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = Buffer->Handle
		};
		Buffer->Address = vkGetBufferDeviceAddress(InContext->Device, &AddressInfo);

		return { Buffer };
	}

	void Buffer::SetData(const void* InData, uint64_t InDataSize)
	{
		InDataSize = std::min(InDataSize, m_Impl->Size);

		VmaAllocationInfo AllocationInfo;
		vmaGetAllocationInfo(m_Impl->Ctx->Allocator, m_Impl->Allocation, &AllocationInfo);
		memcpy(AllocationInfo.pMappedData, InData, InDataSize);
	}

	uint64_t Buffer::GetDeviceAddress()
	{
		return m_Impl->Address;
	}

	void* Buffer::GetMappedMemory()
	{
		VmaAllocationInfo AllocationInfo;
		vmaGetAllocationInfo(m_Impl->Ctx->Allocator, m_Impl->Allocation, &AllocationInfo);
		return AllocationInfo.pMappedData;
	}

	void Buffer::Destroy()
	{
		vmaDestroyBuffer(m_Impl->Ctx->Allocator, m_Impl->Handle, m_Impl->Allocation);
		delete m_Impl;
	}

}
