#pragma once

#include "Engine/RHI/RenderHandles.hpp"

#include <mutex>
#include <condition_variable>

namespace Yuki {

	class TransferManager
	{
		struct TransferQueue
		{
			RHI::Context Context;
			RHI::Queue Queue;
			RHI::CommandPool Pool;
			RHI::Fence SubmitFence;
			DynamicArray<RHI::Buffer> StagingBuffers;
			size_t CurrentStagingBuffer;
			size_t CurrentStagingBufferOffset;
			bool UseHostCopy;
			std::mutex ConditionVarMutex;
			std::condition_variable ConditionVar;
			std::jthread Thread;

			std::mutex JobsMutex;
			DynamicArray<Function<void(RHI::CommandList)>> ScheduledJobs;
			DynamicArray<RHI::Fence> SignalFences;

			RHI::Buffer CopyToStagingBuffer(const std::byte* data, uint64_t dataSize);

			void Execute();
		};

	public:
		TransferManager(RHI::Context context);

		RHI::Image CreateImage(const DynamicArray<std::byte>& data, RHI::ImageInfo imageInfo);
		void UploadImageData(RHI::Image image, const DynamicArray<std::byte>& data);

		RHI::Buffer CreateBuffer(const std::byte* data, uint64_t size, RHI::BufferUsage usage, RHI::BufferFlags flags = RHI::BufferFlags::None);

		template<typename T>
		RHI::Buffer CreateBuffer(Span<T> data, RHI::BufferUsage usage, RHI::BufferFlags flags = RHI::BufferFlags::None)
		{
			const std::byte* byteData = reinterpret_cast<const std::byte*>(data.Data());
			return CreateBuffer(byteData, data.ByteSize(), usage, flags);
		}

		RHI::Buffer CreateBuffer(uint64_t size, RHI::BufferUsage usage, RHI::BufferFlags flags = RHI::BufferFlags::None)
		{
			return CreateBuffer(nullptr, size, usage, flags);
		}

		void UploadBufferData(RHI::Buffer buffer, const std::byte* data, uint64_t dataSize, uint32_t offset = 0);
		
		template<typename T>
		void UploadBufferData(RHI::Buffer buffer, Span<T> data, uint32_t startIndex = 0)
		{
			const std::byte* byteData = reinterpret_cast<const std::byte*>(data.Data());
			UploadBufferData(buffer, byteData, data.ByteSize(), startIndex * sizeof(T));
		}

		void Execute(Span<RHI::Fence> fences);

	private:
		RHI::Context m_Context;
		DynamicArray<Unique<TransferQueue>> m_TransferQueues;
	};
}
