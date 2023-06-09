#pragma once

#include "Yuki/Core/JobSystem.hpp"
#include "Yuki/Core/StableDynamicArray.hpp"
#include "Yuki/Core/InitializerList.hpp"

#include "RHI.hpp"

namespace Yuki {

	using TransferJobFunc = std::function<void(CommandPoolHandle)>;
	using JobFinishCallback = std::function<void()>;

	class RenderContext;

	class TransferScheduler
	{
	public:
		TransferScheduler(RenderContext* InContext);

		void Schedule(TransferJobFunc InJobFunc, JobFinishCallback InFinishedCallback);
		void SubmitCommandBuffer(size_t InThreadID, CommandListHandle InCommandList, InitializerList<FenceHandle> InWaits, InitializerList<FenceHandle> InSignals);
		void Execute();

	private:
		RenderContext* m_Context = nullptr;

		JobSystem m_JobSystem;

		DynamicArray<CommandPoolHandle> m_CommandPools;
		StableDynamicArray<Job> m_Jobs;

		std::shared_mutex m_Mutex;
		DynamicArray<Job*> m_ActiveJobs;

		Barrier m_Barrier{};

		FenceHandle m_Fence{};

	};

}
