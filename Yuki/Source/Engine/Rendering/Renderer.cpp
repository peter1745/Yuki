#include "Rendering/Renderer.hpp"

namespace Yuki {

	Renderer::Renderer(RenderContext* InContext)
		: m_Context(InContext)
	{
		m_JobSystem.Init(2);
		m_CommandPools.resize(2);
		m_StagingBuffers.resize(2);

		m_CommandPools[0] = CommandPool(InContext, InContext->GetTransferQueue(0));
		m_CommandPools[1] = CommandPool(InContext, InContext->GetTransferQueue(1));

		m_StagingBuffers[0] = Buffer(InContext, {
			.Type = BufferType::StagingBuffer,
			.Size = 100 * 1024 * 1024
		});

		m_StagingBuffers[1] = Buffer(InContext, {
			.Type = BufferType::StagingBuffer,
			.Size = 100 * 1024 * 1024
		});

		m_Fence = Fence(InContext);
	}

	void Renderer::ScheduleTransfer(TransferFunc&& InTransferFunc, Barrier* InBarrier)
	{
		auto [jobIndex, job] = m_Jobs.EmplaceBack();
		job.Task = [&, jobFunc = std::move(InTransferFunc)](size_t InThreadID) mutable
		{
			auto transferQueue = Queue{ m_Context->GetTransferQueue(InThreadID), m_Context };
			jobFunc(transferQueue, m_CommandPools[InThreadID], m_StagingBuffers[InThreadID], m_Fence);
		};
		job.OnDone = [this, j = &job]() mutable
		{
			std::scoped_lock lock(m_Mutex);
			std::erase(m_ActiveJobs, j);
		};

		if (InBarrier)
			job.AddSignal(InBarrier);

		std::scoped_lock lock(m_Mutex);
		m_ActiveJobs.push_back(&job);
	}

	void Renderer::ExecuteTransfers()
	{
		m_Barrier.Wait();
		m_Fence.Wait();

		m_CommandPools[0].Reset();
		m_CommandPools[1].Reset();

		std::scoped_lock lock(m_Mutex);
		for (auto* job : m_ActiveJobs)
		{
			job->AddSignal(&m_Barrier);
			m_JobSystem.Schedule(job);
		}
	}

}
