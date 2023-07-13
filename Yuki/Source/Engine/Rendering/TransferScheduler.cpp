#include "Rendering/TransferScheduler.hpp"
#include "Rendering/RenderResources.hpp"

namespace Yuki {

	TransferScheduler::TransferScheduler(RenderContext* InContext)
		: m_Context(InContext)
	{
		m_JobSystem.Init(2);
		m_CommandPools.resize(2);

		m_CommandPools[0] = CommandPool(InContext, InContext->GetTransferQueue(0));
		m_CommandPools[1] = CommandPool(InContext, InContext->GetTransferQueue(1));

		m_Fence = Fence(InContext);
	}

	void TransferScheduler::Schedule(TransferJobFunc InJobFunc, JobFinishCallback InFinishedCallback)
	{
		auto[jobIndex, job] = m_Jobs.EmplaceBack();
		job.Task = [&, jobFunc = std::move(InJobFunc)](size_t InThreadID) mutable
		{
			jobFunc(m_CommandPools[InThreadID]);
		};
		job.OnDone = [this, j = &job, callback = std::move(InFinishedCallback)]() mutable
		{
			std::scoped_lock lock(m_Mutex);
			callback();
			std::erase(m_ActiveJobs, j);
		};

		std::scoped_lock lock(m_Mutex);
		m_ActiveJobs.push_back(&job);
	}

	void TransferScheduler::SubmitCommandBuffer(size_t InThreadID, CommandListHandle InCommandList, InitializerList<FenceHandle> InWaits, InitializerList<FenceHandle> InSignals)
	{
		auto waits = DynamicArray<FenceHandle>(InWaits.List);
		auto signals = DynamicArray<FenceHandle>(InSignals.List);

		waits.emplace_back(m_Fence);
		signals.emplace_back(m_Fence);

		Queue(m_Context->GetTransferQueue(InThreadID), m_Context).SubmitCommandLists({ InCommandList }, waits, signals);
	}

	void TransferScheduler::Execute()
	{
		m_Barrier.Wait();

		m_Context->FenceWait(m_Fence);

		m_Context->CommandPoolReset(m_CommandPools[0]);
		m_Context->CommandPoolReset(m_CommandPools[1]);

		std::scoped_lock lock(m_Mutex);
		for (auto* job : m_ActiveJobs)
		{
			job->AddSignal(&m_Barrier);
			m_JobSystem.Schedule(job);
		}
	}

}
