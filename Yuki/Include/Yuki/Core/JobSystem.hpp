#pragma once

#include <concurrentqueue/concurrentqueue.h>

#include <thread>
#include <vector>
#include <functional>
#include <shared_mutex>

namespace Yuki {

	using JobFunction = std::function<void(size_t)>;
	using DoneCallback = std::function<void()>;

	struct Job;
	struct Barrier
	{
		std::atomic<uint32_t> Counter = 0;
		std::vector<Job*> Pending;

		Barrier() = default;
		Barrier(const Barrier& InOther)
			: Counter(InOther.Counter.load()), Pending(InOther.Pending)
		{
		}

		void Wait(uint32_t InValue = 0) const
		{
			uint32_t value = Counter.load();
			while (value != InValue)
			{
				Counter.wait(value);
				value = Counter.load();
			}
		}
	};

	struct Job
	{
		JobFunction Task;
		std::vector<Barrier*> Signals;
		DoneCallback OnDone;

		void AddSignal(Barrier* InBarrier)
		{
			InBarrier->Counter++;
			Signals.emplace_back(std::move(InBarrier));
		}
	};

	class JobSystem
	{
	public:
		JobSystem() = default;
		JobSystem(uint32_t InThreadCount);
		~JobSystem();

		void Init(uint32_t InThreadCount);

		void Schedule(Job* InJob);

		void SetWorkerName(std::jthread& InThread, std::string_view InName);

	private:
		struct WorkerThreadData
		{
			size_t ThreadID;
			moodycamel::ProducerToken ProducerToken;
			moodycamel::ConsumerToken ConsumerToken;
		};

		void WorkerThread(WorkerThreadData InWorkerData);
		void ScheduleWithProducer(moodycamel::ProducerToken& InToken, Job* InJob);

		void Shutdown();

	private:
		bool m_ExecuteJobs = true;
		
		moodycamel::ConcurrentQueue<Job*> m_Jobs;
		std::mutex m_GlobalMutex;
		std::vector<std::jthread> m_WorkerThreads;
		std::condition_variable_any m_ConditionVariable;
	};

}
