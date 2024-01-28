#pragma once

#include <cstdint>

#include <Aura/Unique.hpp>
#include <Aura/Arena.hpp>

namespace Yuki {

	template<typename T>
	concept HandleType = requires(T t)
	{
		{ T::Create() } -> std::same_as<T>;
		{ t.Destroy() };
		{ t.Unwrap() } -> std::same_as<T>;
		{ t.operator->() } -> std::same_as<typename T::Impl*>;
	};

	struct HandleControlBlock
	{
		uint32_t RefCount;
		uint32_t WeakReferences;
	};

	struct ControlBlockAllocator
	{
		inline static Aura::Unique<Aura::Arena> s_ControlBlockArena = Aura::Unique<Aura::Arena>::New(1 * 1024 * 1024);

		Aura::Arena::Block AllocateControlBlock() const
		{
			return s_ControlBlockArena->Allocate(sizeof(HandleControlBlock));
		}

		void DeallocateControlBlock(Aura::Arena::Block controlBlock)
		{
			s_ControlBlockArena->Free(controlBlock);
		}
	};

	template<typename T>
	struct Handle
	{
		using ID = uintptr_t;
		
		struct Impl;

		Handle() = default;
		Handle(Impl* impl)
			: m_Impl(impl)
		{
			m_ControlBlock = m_Allocator.AllocateControlBlock().As<HandleControlBlock>();

			IncreaseRefCount();
		}

		Handle(const Handle& other)
			: m_Impl(other.m_Impl), m_ControlBlock(other.m_ControlBlock)
		{
			IncreaseRefCount();
		}

		~Handle()
		{
			DecreaseRefCount();
		}

		Handle& operator=(const Handle& other)
		{
			if (m_ControlBlock && m_Impl != other.m_Impl)
			{
				DecreaseRefCount();
			}

			m_Impl = other.m_Impl;
			m_ControlBlock = other.m_ControlBlock;

			IncreaseRefCount();
			return *this;
		}

		Handle& operator=(Handle&& other) noexcept
		{
			if (m_Impl == other.m_Impl)
			{
				return *this;
			}

			if (m_ControlBlock)
			{
				m_ControlBlock->WeakReferences--;
			}

			m_Impl = other.m_Impl;
			m_ControlBlock = other.m_ControlBlock;
			return *this;
		}

		T Unwrap() const noexcept { return T(m_Impl); }
		operator T() const noexcept { return Unwrap(); }

		bool IsValid() const noexcept { return m_Impl; }
		bool IsAlive() const { return m_ControlBlock && m_ControlBlock->RefCount > 0; }

		operator bool() const noexcept { return IsValid() && IsAlive(); }
		Impl* operator->() const noexcept { return m_Impl; }

		ID GetID() const noexcept { return reinterpret_cast<ID>(m_Impl); }

	private:
		void IncreaseRefCount()
		{
			if (!m_ControlBlock)
				return;

			m_ControlBlock->WeakReferences++;
		}

		void DecreaseRefCount()
		{
			if (!m_ControlBlock)
				return;

			m_ControlBlock->WeakReferences--;

			if (m_ControlBlock->WeakReferences == 0 && m_ControlBlock->RefCount == 0)
			{
				m_Allocator.DeallocateControlBlock(Aura::Arena::Block::From(m_ControlBlock));
			}
		}
	
	protected:
		Impl* m_Impl = nullptr;

	private:
		AuraNoUniqueAddress ControlBlockAllocator m_Allocator;
		HandleControlBlock* m_ControlBlock = nullptr;

		template<HandleType T>
		friend class SharedHandle;
	};

	template<HandleType T>
	class SharedHandle
	{
	public:
		SharedHandle() = default;

		SharedHandle(T handle)
			: m_Impl(handle.m_Impl), m_ControlBlock(handle.m_ControlBlock)
		{
			IncreaseRefCount();
		}

		SharedHandle(const SharedHandle& other)
			: m_Impl(other.m_Impl), m_ControlBlock(other.m_ControlBlock)
		{
			IncreaseRefCount();
		}

		SharedHandle(SharedHandle&& other) noexcept
		{
			m_Impl = std::exchange(other.m_Impl, nullptr);
			m_ControlBlock = std::exchange(other.m_ControlBlock, nullptr);
		}

		~SharedHandle()
		{
			DecreaseRefCount();
		}

		SharedHandle& operator=(const SharedHandle& other)
		{
			DecreaseRefCount();

			m_Impl = other.m_Impl;
			m_ControlBlock = other.m_ControlBlock;

			IncreaseRefCount();

			return *this;
		}

		SharedHandle& operator=(SharedHandle&& other) noexcept
		{
			if (m_Impl != other.m_Impl)
			{
				DecreaseRefCount();
			}

			m_Impl = std::exchange(other.m_Impl, nullptr);
			m_ControlBlock = std::exchange(other.m_ControlBlock, nullptr);

			return *this;
		}

		T::Impl* operator->() const noexcept { return m_Impl; }

	private:
		void IncreaseRefCount()
		{
			if (m_Impl == nullptr || m_ControlBlock == nullptr)
				return;

			m_ControlBlock->RefCount++;
		}

		void DecreaseRefCount()
		{
			if (m_Impl == nullptr || m_ControlBlock == nullptr)
				return;

			if (--m_ControlBlock->RefCount == 0)
			{
				T{ m_Impl }.Destroy();

				if (m_ControlBlock->WeakReferences == 0)
				{
					m_Allocator.DeallocateControlBlock(Aura::Arena::Block::From(m_ControlBlock));
				}
			}
		}

	private:
		AuraNoUniqueAddress ControlBlockAllocator m_Allocator;
		T::Impl* m_Impl = nullptr;
		HandleControlBlock* m_ControlBlock = nullptr;
	};

}
