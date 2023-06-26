#include <iostream>

//#include "StableDynamicArray.hpp"
//#include "JobSystem.hpp"

int main()
{
//	for (uint32_t j = 0; j < 2; j++)
//	{
//		TestFramework::JobSystem jobSystem(8);
//
//		TestFramework::StableDynamicArray<TestFramework::Job> jobs;
//
//		TestFramework::Barrier barrier2;
//		auto[index, job] = jobs.Acquire();
//		job.Task = [](size_t InThreadID, void* InData)
//		{
//			std::cout << "Running Child Task (" << InThreadID << ")" << std::endl;
//			std::cout << "Child Task Done" << std::endl;
//		};
//		job.AddSignal(&barrier2);
//
//		TestFramework::Barrier barrier;
//		barrier.Pending.push_back(&job);
//
//		for (uint32_t i = 0; i < 50; i++)
//		{
//			auto[index1, dummyJob] = jobs.Acquire();
//			dummyJob.Task = [](size_t InThreadID, void* InData)
//			{
//				for (uint32_t x = 0; x < 1000; x++);
//			};
//			jobSystem.Schedule(&dummyJob);
//		}
//
//		auto[index2, job2] = jobs.Acquire();
//		job2.Task = [](size_t InThreadID, void* InData)
//		{
//			std::cout << "job2 (" << InThreadID << ")" << std::endl;
//		};
//		job2.AddSignal(&barrier);
//		jobSystem.Schedule(&job2);
//
//		barrier.Wait();
//
//		std::cout << "End" << std::endl;
//
//		barrier2.Wait();
//	}
//
	return 0;
}
