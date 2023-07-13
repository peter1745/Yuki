#include "World/World.hpp"
#include "Entities/TransformComponents.hpp"
#include "Entities/RenderingComponents.hpp"
#include "Rendering/EntityRenderer.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

namespace Yuki {

	namespace Layers
	{
		static constexpr JPH::ObjectLayer NON_MOVING = 0;
		static constexpr JPH::ObjectLayer MOVING = 1;
		static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
	};

	class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
	{
	public:
		bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
		{
			switch (inObject1)
			{
			case Layers::NON_MOVING: return inObject2 == Layers::MOVING;
			case Layers::MOVING: return true;
			default:
				YUKI_VERIFY(false);
				return false;
			}
		}
	};

	namespace BroadPhaseLayers
	{
		static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
		static constexpr JPH::BroadPhaseLayer MOVING(1);
		static constexpr uint32_t NUM_LAYERS(2);
	};

	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
			mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		}

		uint32_t GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			YUKI_VERIFY(inLayer < Layers::NUM_LAYERS);
			return mObjectToBroadPhase[inLayer];
		}

	#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
		{
			return "";
		}
	#endif

	private:
		JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
	};

	class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
		{
			switch (inLayer1)
			{
			case Layers::NON_MOVING: return inLayer2 == BroadPhaseLayers::MOVING;
			case Layers::MOVING: return true;	
			default:
				YUKI_VERIFY(false);
				return false;
			}
		}
	};

	static BPLayerInterfaceImpl s_BroadPhaseLayerInterface;
	static ObjectVsBroadPhaseLayerFilterImpl s_ObjectBroadPhaseLayerFitler;
	static ObjectLayerPairFilterImpl s_ObjectVsObjectLayerFilter;

	World::World()
	{
		JPH::RegisterDefaultAllocator();
		JPH::Factory::sInstance = new JPH::Factory();
		JPH::RegisterTypes();

		m_Allocator = Unique<JPH::TempAllocatorImpl>::Create(500 * 1024 * 1024);
		m_ThreadPool = Unique<JPH::JobSystemThreadPool>::Create(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 4);
		m_PhysicsSystem = Unique<JPH::PhysicsSystem>::Create();
		m_PhysicsSystem->Init(65536, 0, 65536, 10240, s_BroadPhaseLayerInterface, s_ObjectBroadPhaseLayerFitler, s_ObjectVsObjectLayerFilter);
	}

	flecs::entity World::CreateEntity(std::string_view InName)
	{
		auto entity = m_EntityWorld.entity();
		entity.emplace<Entities::Name>(InName);
		entity.emplace<Entities::Translation>();
		entity.emplace<Entities::Rotation>();
		entity.emplace<Entities::Scale>(Math::Vec3{1.0f, 1.0f, 1.0f});
		
		JPH::BoxShapeSettings shapeSettings({ 0.5f, 0.5f, 0.5f });
		auto shape = shapeSettings.Create();
		JPH::BodyCreationSettings bodySettings(shape.Get(), { 0.0f, 0.0f, 0.0f }, JPH::QuatArg::sIdentity(), JPH::EMotionType::Dynamic, Layers::MOVING);
		m_PhysicsBodies[entity] = m_PhysicsSystem->GetBodyInterface().CreateBody(bodySettings);

		m_EntityCreatedCallback(entity);
		return entity;
	}

	flecs::entity World::InstantiateMeshScene(AssetID InMeshID, const MeshScene& InScene)
	{
		return CreateMeshHierarchy(InMeshID, InScene, InScene.RootNodeIndex, flecs::entity::null());
	}

	flecs::entity World::CreateMeshHierarchy(AssetID InMeshID, const MeshScene& InScene, size_t InNodeIndex, flecs::entity InParentEntity)
	{
		const auto& node = InScene.Nodes[InNodeIndex];
		auto entity = CreateEntity(node.Name);

		entity.get_mut<Entities::Translation>()->Value = node.Translation;
		entity.get_mut<Entities::Rotation>()->Value = node.Rotation;
		entity.get_mut<Entities::Scale>()->Value = node.Scale;

		if (node.MeshIndex != -1)
		{
			entity.emplace<Entities::GPUTransform>(0U);
			entity.set([InMeshID, meshIndex = node.MeshIndex](Entities::MeshComponent& InMeshComponent)
			{
				InMeshComponent.MeshID = InMeshID;
				InMeshComponent.MeshIndex = meshIndex;
			});
		}

		if (InParentEntity != flecs::entity::null())
			entity.child_of(InParentEntity);

		for (auto childNodeIndex : node.ChildNodes)
			YUKI_UNUSED(CreateMeshHierarchy(InMeshID, InScene, childNodeIndex, entity));

		return entity;
	}

	void World::Tick(float InDeltaTime)
	{
		m_EntityWorld.progress(InDeltaTime);

		/*if (m_Simulating)
		{
			nanoseconds tickPeriod = duration_cast<std::chrono::nanoseconds>(1s) / 120;
			float fixedTimeStep = duration_cast<std::chrono::duration<float>>(tickPeriod).count();
			auto elapsed = steady_clock::now() - m_StartTime;

			while (m_SimulatedTime < elapsed)
			{
				m_SimulatedTime += tickPeriod;
				m_PhysicsSystem->Update(fixedTimeStep, 1, m_Allocator, m_ThreadPool);
			}

			for (const auto&[entity, body] : m_PhysicsBodies)
			{
				const auto& position = body->GetPosition();
				auto* translation = entity.get_mut<Entities::Translation>();
				translation->Value.X = position.GetX();
				translation->Value.Y = position.GetY();
				translation->Value.Z = position.GetZ();

				m_Renderer->SynchronizeGPUTransform(entity);
			}
		}*/
	}

	void World::StartSimulation()
	{
		DynamicArray<JPH::BodyID> bodyIDs;
		bodyIDs.reserve(m_PhysicsBodies.size());
		for (const auto&[entity, body] : m_PhysicsBodies)
			bodyIDs.push_back(body->GetID());

		auto state = m_PhysicsSystem->GetBodyInterface().AddBodiesPrepare(bodyIDs.data(), bodyIDs.size());
		m_PhysicsSystem->GetBodyInterface().AddBodiesFinalize(bodyIDs.data(), bodyIDs.size(), state, JPH::EActivation::Activate);

		m_Simulating = true;

		m_StartTime = steady_clock::now();
		m_SimulatedTime = 0ns;
	}

	void World::StopSimulation()
	{
	}

}
