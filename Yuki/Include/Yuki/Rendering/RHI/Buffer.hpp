#pragma once

namespace Yuki {

	enum class BufferType { VertexBuffer, IndexBuffer, StorageBuffer, StagingBuffer };

	struct BufferInfo
	{
		BufferType Type;
		uint32_t Size;
		bool PersistentlyMapped = false;
	};

	struct BufferUploadInfo
	{
		uint32_t SrcOffset = 0;
		uint32_t DstOffset = 0;
		uint32_t Size = 0;
	};

	class Buffer
	{
	public:
		virtual ~Buffer() = default;

		virtual void SetData(const void* InData, uint32_t InDataSize, uint32_t InDstOffset = 0) = 0;

		virtual const BufferInfo& GetInfo() const = 0;

	};

}
