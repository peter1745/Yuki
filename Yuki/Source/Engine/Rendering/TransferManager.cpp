#include "TransferManager.hpp"

namespace Yuki {

	// 10mb for now
	static constexpr size_t HeavyTransferThreshold = 10 * 1024 * 1024;
	static constexpr uint64_t DefaultStagingBufferSize = 50 * 1024 * 1024;

	TransferManager::TransferManager(RHI::Context context)
		: m_Context(context)
	{
		bool useHostCopy = m_Context.IsFeatureEnabled(RHI::RendererFeature::HostImageCopy);
		
		auto transferQueues = m_Context.RequestQueues(RHI::QueueType::Transfer, 2);
		YUKI_VERIFY(!transferQueues.empty());

		for (auto queue : transferQueues)
		{
			auto& transferQueue = m_TransferQueues.emplace_back();
			transferQueue.Context = m_Context;
			transferQueue.Queue = queue;

			if (!useHostCopy)
			{
				transferQueue.Pool = RHI::CommandPool::Create(m_Context, queue);
				transferQueue.SubmitFence = RHI::Fence::Create(m_Context);
			}

			transferQueue.UseHostCopy = useHostCopy;
			transferQueue.CurrentStagingBuffer = 0;
			transferQueue.CurrentStagingBufferOffset = 0;
		}
	}


	void TransferManager::Execute(Span<RHI::Fence> fences)
	{
		for (auto& transferQueue : m_TransferQueues)
			transferQueue.Execute(fences);
	}

	RHI::Buffer TransferManager::TransferQueue::CopyToStagingBuffer(const std::byte* data, uint64_t dataSize)
	{
		auto createStagingBuffer = [this](uint64_t size)
		{
			return RHI::Buffer::Create(
				Context,
				size,
				RHI::BufferUsage::TransferDst |
				RHI::BufferUsage::TransferSrc,
				RHI::BufferFlags::Mapped);
		};

		if (StagingBuffers.empty()) [[unlikely]]
		{
			StagingBuffers.push_back(createStagingBuffer(DefaultStagingBufferSize));
		}

		uint64_t bufferSize = StagingBuffers[CurrentStagingBuffer].GetSize();

		// Check if the current buffer can hold data at the given offset, if not create a new one that can
		if (CurrentStagingBufferOffset + dataSize >= bufferSize)
		{
			uint64_t requiredSize = bufferSize + (bufferSize / 2) + dataSize;
			Logging::Info("Allocating staging buffer with size = {}", requiredSize);
			StagingBuffers.push_back(createStagingBuffer(requiredSize));
			CurrentStagingBuffer = StagingBuffers.size() - 1;
			CurrentStagingBufferOffset = 0;
		}

		auto& currentBuffer = StagingBuffers[CurrentStagingBuffer];
		currentBuffer.SetData(data, dataSize, CurrentStagingBufferOffset);
		CurrentStagingBufferOffset += dataSize;
		return currentBuffer;
	}

	RHI::Image TransferManager::CreateImage(const DynamicArray<std::byte>& data, RHI::ImageInfo imageInfo)
	{
		uint32_t queueIndex = data.size() < HeavyTransferThreshold ? 0 : 1;
		auto& transferQueue = m_TransferQueues[queueIndex];
		if (transferQueue.UseHostCopy)
		{
			imageInfo.Usage |= RHI::ImageUsage::HostTransfer;
		}
		else
		{
			imageInfo.Usage |= RHI::ImageUsage::TransferDest;
		}

		auto image = RHI::Image::Create(m_Context, imageInfo);
		UploadImageData(image, data);

		return image;
	}

	void TransferManager::UploadImageData(RHI::Image image, const DynamicArray<std::byte>& data)
	{
		uint32_t queueIndex = data.size() < HeavyTransferThreshold ? 0 : 1;
		auto& transferQueue = m_TransferQueues[queueIndex];

		transferQueue.ScheduledJobs.push_back([&, image, data](RHI::CommandList cmd)
		{
			if (transferQueue.UseHostCopy)
			{
				image.SetData(data.data());
			}
			else
			{
				auto buffer = transferQueue.CopyToStagingBuffer(data.data(), data.size());
				uint32_t startOffset = transferQueue.CurrentStagingBufferOffset - data.size();

				cmd.ImageBarrier({
					.Images = { image },
					.Layouts = { RHI::ImageLayout::TransferDest }
				});
				cmd.CopyBufferToImage(image, buffer, startOffset);
				cmd.ImageBarrier({
					.Images = { image },
					.Layouts = { RHI::ImageLayout::General }
				});
			}
		});
	}

	RHI::Buffer TransferManager::CreateBuffer(const std::byte* data, uint64_t size, RHI::BufferUsage usage, RHI::BufferFlags flags)
	{
		Logging::Info("Creating buffer with size {}", size);
		uint32_t queueIndex = size < HeavyTransferThreshold ? 0 : 1;
		auto& transferQueue = m_TransferQueues[queueIndex];

		if (transferQueue.UseHostCopy)
		{
			usage |= RHI::BufferUsage::TransferDst;
			flags |= RHI::BufferFlags::Mapped;
			flags |= RHI::BufferFlags::DeviceLocal;
		}
		else
		{
			usage |= RHI::BufferUsage::TransferDst;
		}

		auto result = RHI::Buffer::Create(m_Context, size, usage, flags);

		if (data != nullptr)
			UploadBufferData(result, data, size);

		return result;
	}

	void TransferManager::UploadBufferData(RHI::Buffer buffer, const std::byte* data, uint64_t dataSize, uint32_t offset)
	{
		uint32_t queueIndex = dataSize < HeavyTransferThreshold ? 0 : 1;
		auto& transferQueue = m_TransferQueues[queueIndex];

		DynamicArray<std::byte> dataCopy(data, data + Cast<size_t>(dataSize));

		transferQueue.ScheduledJobs.push_back([&, buffer, dataCopy, dataSize, offset](RHI::CommandList cmd) mutable
		{
			if (transferQueue.UseHostCopy)
			{
				buffer.SetData(dataCopy.data(), dataSize, offset);
			}
			else
			{
				auto srcBuffer = transferQueue.CopyToStagingBuffer(dataCopy.data(), dataSize);
				uint32_t startOffset = transferQueue.CurrentStagingBufferOffset - dataSize;
				cmd.CopyBuffer(buffer, offset, srcBuffer, transferQueue.CurrentStagingBufferOffset);
			}
		});
	}

	void TransferManager::TransferQueue::Execute(Span<RHI::Fence> fences)
	{
		if (ScheduledJobs.empty())
			return;

		Logging::Info("Executing {} transfers", ScheduledJobs.size());

		if (UseHostCopy)
		{
			for (auto&& func : ScheduledJobs)
				func({});
		}
		else
		{
			SubmitFence.Wait();
			Pool.Reset();

			auto cmd = Pool.NewList();
			cmd.Begin();
			for (auto&& func : ScheduledJobs)
			{
				func(cmd);
			}
			cmd.End();

			DynamicArray<RHI::Fence> fencesArr(fences.Data(), fences.Data() + fences.Count());
			fencesArr.push_back(SubmitFence);
			Queue.Submit({ cmd }, {}, fencesArr);

			CurrentStagingBuffer = 0;
			CurrentStagingBufferOffset = 0;
		}

		ScheduledJobs.clear();
	}

}
