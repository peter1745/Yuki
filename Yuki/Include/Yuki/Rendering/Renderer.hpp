#pragma once

#include "RenderResources.hpp"

#include "Yuki/Core/JobSystem.hpp"
#include "Yuki/Core/StableDynamicArray.hpp"

namespace Yuki {

	class Renderer
	{
	public:
		using TransferFunc = std::function<void(Queue, CommandPool, Buffer, Fence)>;

	public:
		Renderer(RenderContext* InContext);
		virtual ~Renderer() = default;

	public:
		void ScheduleTransfer(TransferFunc&& InTransferFunc, Barrier* InBarrier = nullptr);
		void ExecuteTransfers();

	protected:
		RenderContext* m_Context = nullptr;

	private:
		JobSystem m_JobSystem;
		StableDynamicArray<Job> m_Jobs;
		std::shared_mutex m_Mutex;
		DynamicArray<Job*> m_ActiveJobs;

		DynamicArray<Buffer> m_StagingBuffers;

		DynamicArray<CommandPool> m_CommandPools;

		Barrier m_Barrier{};

		Fence m_Fence{};
	};

}
