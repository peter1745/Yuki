#pragma once

#include "Yuki/Asset/AssetID.hpp"
#include "Yuki/Rendering/Mesh.hpp"

#include <flecs/flecs.h>

#include <chrono>

namespace JPH {

	class TempAllocator;
	class JobSystemThreadPool;
	class PhysicsSystem;
	class Body;
}

namespace Yuki {

	struct FlecsIDHash
	{
		using is_avalanching = void;

		uint64_t operator()(const flecs::id& InID) const noexcept
		{
			return ankerl::unordered_dense::detail::wyhash::hash(&InID, sizeof(flecs::id));
		}
	};

	struct FlecsEntityHash
	{
		using is_avalanching = void;

		uint64_t operator()(const flecs::entity& InEntity) const noexcept
		{
			return ankerl::unordered_dense::detail::wyhash::hash(&InEntity, sizeof(flecs::entity));
		}
	};

	class WorldRenderer;

	using OnEntityCreateFn = std::function<void(flecs::entity)>;

	class World
	{
	public:
		World();

		void SetOnEntityCreateCallback(OnEntityCreateFn&& InCallback)
		{
			m_EntityCreatedCallback = std::move(InCallback);
		}

		flecs::entity CreateEntity(std::string_view InName);
		flecs::entity InstantiateMeshScene(AssetID InMeshID, const MeshScene& InScene);

		void Tick(float InDeltaTime);

		void StartSimulation();
		void StopSimulation();

		flecs::world& GetEntityWorld() { return m_EntityWorld; }

		template<typename TFunc>
		void IterateHierarchy(flecs::entity InEntity, TFunc&& InFunc)
		{
			InFunc(InEntity);

			InEntity.children([this, &InFunc](flecs::entity InChild)
			{
				IterateHierarchy(InChild, InFunc);
			});
		}

	private:
		flecs::entity CreateMeshHierarchy(AssetID InMeshID, const MeshScene& InScene, size_t InNodeIndex, flecs::entity InParentEntity);

	private:
		flecs::world m_EntityWorld;

		OnEntityCreateFn m_EntityCreatedCallback;

		Unique<JPH::TempAllocator> m_Allocator = nullptr;
		Unique<JPH::JobSystemThreadPool> m_ThreadPool = nullptr;
		Unique<JPH::PhysicsSystem> m_PhysicsSystem = nullptr;

		Map<flecs::entity, JPH::Body*, FlecsEntityHash> m_PhysicsBodies;

		bool m_Simulating = false;

		steady_clock::time_point m_StartTime;
		nanoseconds m_SimulatedTime;
	};

}
