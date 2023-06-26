#include "Core/JobSystem.hpp"

#if defined(YUKI_PLATFORM_WINDOWS)
	#include "Platform/Windows/WindowsUtils.hpp"
#endif

namespace Yuki {

	JobSystem::JobSystem(uint32_t InThreadCount)
	{
		Init(InThreadCount);
	}

	JobSystem::~JobSystem()
	{
		Shutdown();
	}

	void JobSystem::Init(uint32_t InThreadCount)
	{
		for (uint32_t i = 0; i < InThreadCount; i++)
		{
			WorkerThreadData workerData =
			{
				.ThreadID = m_WorkerThreads.size(),
				.ProducerToken = moodycamel::ProducerToken(m_Jobs),
				.ConsumerToken = moodycamel::ConsumerToken(m_Jobs)
			};

			std::string name = fmt::format("Worker-{}", i);
			auto& worker = m_WorkerThreads.emplace_back(&JobSystem::WorkerThread, this, std::move(workerData));
			SetWorkerName(worker, name);
		}
	}

	void JobSystem::SetWorkerName(std::jthread& InThread, std::string_view InName)
	{
		if constexpr (s_Platform == Platform::Windows)
		{
			auto name = WindowsUtils::ConvertUtf8ToWide(InName);
			SetThreadDescription(InThread.native_handle(), name.c_str());
		}
	}

	void JobSystem::Shutdown()
	{
		m_ExecuteJobs = false;
		m_ConditionVariable.notify_all();
	}

	void JobSystem::Schedule(Job* InJob)
	{
		m_Jobs.enqueue(InJob);
		m_ConditionVariable.notify_one();
	}

	void JobSystem::ScheduleWithProducer(moodycamel::ProducerToken& InToken, Job* InJob)
	{
		m_Jobs.enqueue(InToken, InJob);
		m_ConditionVariable.notify_one();
	}

	void JobSystem::WorkerThread(WorkerThreadData InWorkerData)
	{
		while (true)
		{
			Job* job = nullptr;

			uint32_t spins = 1;
			do {
				m_Jobs.try_dequeue_from_producer(InWorkerData.ProducerToken, job);
			} while (!job && spins--);

			if (!job)
			{
				std::unique_lock lock(m_GlobalMutex);

				while (m_Jobs.try_dequeue(InWorkerData.ConsumerToken, job), !job)
				{
					if (!m_ExecuteJobs)
						return;

					m_ConditionVariable.wait(lock);
				}
			}

			job->Task(InWorkerData.ThreadID);
			
			for (auto* barrier : job->Signals)
			{
				if (--barrier->Counter == 0)
				{
					for (auto* pending : barrier->Pending)
						ScheduleWithProducer(InWorkerData.ProducerToken, pending);

					barrier->Counter.notify_all();
				}
			}

			if (job->OnDone)
				job->OnDone();
		}
	}

}
