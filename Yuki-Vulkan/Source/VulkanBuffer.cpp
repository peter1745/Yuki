#include "VulkanRHI.hpp"

namespace Yuki {

	Buffer Buffer::Create(RHIContext context, uint64_t size, BufferUsage usage)
	{
		auto* impl = new Impl();
		impl->Context = context;

		VkBufferUsageFlags2CreateInfoKHR usageFlags =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO_KHR,
			.usage = BufferUsageToVkBufferUsage(usage),
		};

		VkBufferCreateInfo bufferInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = &usageFlags,
			.size = size,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};

		impl->Allocation = context->Allocator.CreateBuffer(bufferInfo, usage);
		impl->Size = size;

		VkBufferDeviceAddressInfo addressInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.buffer = impl->Allocation.Resource
		};
		impl->Address = vkGetBufferDeviceAddress(context->Device, &addressInfo);

		return { impl };
	}

	void Buffer::Destroy()
	{
		m_Impl->Context->Allocator.DestroyBuffer(m_Impl->Allocation);
		delete m_Impl;
	}

	uint64_t Buffer::GetAddress() const { return m_Impl->Address; }

	void Buffer::SetData(std::byte* data, uint32_t offset, uint32_t size) const
	{
		std::byte* memory = reinterpret_cast<std::byte*>(m_Impl->Allocation.AllocationInfo.pMappedData);
		memcpy(memory + offset, data, size);
	}

}
