#pragma once

#include <flecs/flecs.h>

#include <chrono>

namespace JPH {

	class TempAllocator;
	class JobSystemThreadPool;
	class PhysicsSystem;
	class Body;
}

namespace Yuki {

	struct FlecsEntityHash
	{
		using is_avalanching = void;

		uint64_t operator()(const flecs::entity& InEntity) const noexcept
		{
			return ankerl::unordered_dense::detail::wyhash::hash(&InEntity, sizeof(flecs::entity));
		}
	};

	class WorldRenderer;

	class World
	{
	public:
		World() = default;
		World(WorldRenderer* InRenderer);

		flecs::entity CreateEntity(std::string_view InName);

		void Tick(float InDeltaTime);

		void StartSimulation();
		void StopSimulation();

	private:
		WorldRenderer* m_Renderer = nullptr;
		flecs::world m_EntityWorld;

		Unique<JPH::TempAllocator> m_Allocator = nullptr;
		Unique<JPH::JobSystemThreadPool> m_ThreadPool = nullptr;
		Unique<JPH::PhysicsSystem> m_PhysicsSystem = nullptr;

		Map<flecs::entity, JPH::Body*, FlecsEntityHash> m_PhysicsBodies;

		bool m_Simulating = false;

		steady_clock::time_point m_StartTime;
		nanoseconds m_SimulatedTime;
	};

}
