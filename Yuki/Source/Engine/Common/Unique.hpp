#pragma once

#include <utility>

namespace Yuki {

	template<typename T>
	struct DefaultDeleter
	{
		void operator()(T* InInstance) const
		{
			delete InInstance;
		}
	};

	template<typename T, typename TDeleter = DefaultDeleter<T>>
	class Unique
	{
	public:
		Unique() = default;

		Unique(T* InInstance) noexcept
			: m_Instance(InInstance) {}

		Unique(Unique&& InOther) noexcept
			: m_Instance(std::exchange(InOther.m_Instance, nullptr))
		{
		}

		template<typename U>
		Unique(Unique<U>&& InOther) noexcept
			: m_Instance(static_cast<T*>(std::exchange(InOther.m_Instance, nullptr)))
		{
		}

		Unique& operator=(Unique&& InOther) noexcept
		{
			m_Instance = std::exchange(InOther.m_Instance, nullptr);
			return *this;
		}

		Unique& operator=(T* InPointer)
		{
			auto* Old = std::exchange(m_Instance, InPointer);

			if (Old)
				TDeleter()(Old);

			return *this;
		}

		template<typename U>
		Unique& operator=(Unique<U>&& InOther) noexcept
		{
			m_Instance = static_cast<T*>(std::exchange(InOther.m_Instance, nullptr));
		}

		Unique(const Unique&) = delete;
		Unique& operator=(const Unique&) = delete;

		~Unique() noexcept
		{
			if (m_Instance)
				TDeleter()(m_Instance);
		}

	public:
		T* Get() const { return m_Instance; }

		T* Release()
		{
			return std::exchange(m_Instance, nullptr);
		}

		operator T&() const & { return *m_Instance; }
		T* operator->() const { return m_Instance; }

		template<typename TOther>
		operator TOther& () const& { return *static_cast<TOther*>(m_Instance); }

	public:
		template<typename... TArgs>
		static Unique<T> New(TArgs&&... InArgs)
		{
			return Unique<T>(new T(std::forward<TArgs>(InArgs)...));
		}

	private:
		T* m_Instance = nullptr;

	private:
		template<typename TOther, typename TOtherDeleter>
		friend class Unique;
	};

}
