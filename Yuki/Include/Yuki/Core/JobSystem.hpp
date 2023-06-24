#pragma once

#include <functional>
#include <queue>
#include <thread>
#include <shared_mutex>

#include "EnumFlags.hpp"

namespace Yuki {

	enum class JobFlags
	{
		None = 0,
		RescheduleOnFinish = 1 << 0
	};
	YUKI_ENUM_FLAGS(JobFlags);

	struct Job
	{
	public:
		using FuncType = std::function<void(size_t)>;

	public:
		void Wait() const { m_Done.wait(false); }
		bool IsDone() const { return m_Done.load(); }

	private:
		FuncType m_JobFunc;
		std::atomic_bool m_Done = false;
		JobFlags m_Flags = JobFlags::None;

		friend class JobSystem;
	};

	class JobSystem
	{
	public:

	public:
		JobSystem() = default;
		JobSystem(uint32_t InThreads);
		~JobSystem();

		void Init(uint32_t InThreads);
		void Shutdown();

		Job* Submit(Job::FuncType&& InJobFunc, JobFlags InFlags = JobFlags::None);
		void SubmitAndWait(Job::FuncType&& InJobFunc, JobFlags InFlags = JobFlags::None);

		void WaitAll() const;

	private:
		void WorkerThread(size_t InWorkerID);

	private:
		std::vector<std::jthread> m_WorkerThreads;
		std::shared_mutex m_JobQueueMutex;
		std::queue<Job*> m_JobQueue;
		std::shared_mutex m_Mutex;
		std::condition_variable_any m_ConditionVariable;

		std::shared_mutex m_FinishedJobsMutex;
		std::queue<Job*> m_FinishedJobs;

		bool m_Running;
	};

}
