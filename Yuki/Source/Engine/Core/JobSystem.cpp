#include "Core/JobSystem.hpp"

namespace Yuki {

	JobSystem::JobSystem(uint32_t InThreads)
	{
		Init(InThreads);
	}

	JobSystem::~JobSystem()
	{
		Shutdown();
	}

	void JobSystem::Init(uint32_t InThreads)
	{
		m_Running = true;
		
		for (uint32_t i = 0; i < InThreads; i++)
		{
			m_WorkerThreads.emplace_back([&]()
			{
				WorkerThread(i);
			});
		}
	}

	void JobSystem::Shutdown()
	{
		{
			std::scoped_lock lock(m_Mutex);
			m_Running = false;
			m_ConditionVariable.notify_all();

			while (!m_FinishedJobs.empty())
			{
				Job* job = m_FinishedJobs.front();
				m_FinishedJobs.pop();
				delete job;
			}
		}
		
		{
			std::scoped_lock lock(m_JobQueueMutex);

			while (!m_JobQueue.empty())
			{
				auto* job = m_JobQueue.back();
				m_JobQueue.pop();
				delete job;
			}
		}
	}

	Job* JobSystem::Submit(Job::FuncType&& InJobFunc, JobFlags InFlags)
	{
		Job* job = nullptr;
		if (!m_FinishedJobs.empty())
		{
			std::scoped_lock lock(m_FinishedJobsMutex);
			job = m_FinishedJobs.front();
			m_FinishedJobs.pop();
		}
		else
		{
			job = new Job();
		}

		std::scoped_lock lock(m_JobQueueMutex);
		job->m_JobFunc = std::move(InJobFunc);
		job->m_Done = false;
		job->m_Flags = InFlags;
		m_JobQueue.push(job);
		m_ConditionVariable.notify_one();

		return job;
	}

	void JobSystem::SubmitAndWait(Job::FuncType&& InJobFunc, JobFlags InFlags)
	{
		const auto* job = Submit(std::move(InJobFunc));
		job->Wait();
	}

	void JobSystem::WaitAll() const
	{
		while (!m_JobQueue.empty())
		{
			const Job* job = m_JobQueue.front();
			job->Wait();
		}
	}

	void JobSystem::WorkerThread(size_t InWorkerID)
	{
		while (true)
		{
			std::unique_lock lock(m_Mutex);

			while (m_JobQueue.empty())
			{
				if (!m_Running)
					return;

				m_ConditionVariable.wait(lock);
			}

			if (!m_Running)
				return;

			auto* job = m_JobQueue.front();
			m_JobQueue.pop();

			lock.unlock();

			job->m_JobFunc(InWorkerID);

			if (job->m_Flags & JobFlags::RescheduleOnFinish)
			{
				std::scoped_lock jobQueueLock(m_JobQueueMutex);
				m_JobQueue.push(job);
			}
			else
			{
				job->m_Done = true;
				job->m_Done.notify_all();
				
				std::scoped_lock finishedLock(m_FinishedJobsMutex);
				m_FinishedJobs.push(job);
			}
		}
	}

}
