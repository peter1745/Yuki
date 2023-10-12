#pragma once

#include <utility>

namespace Yuki {

	template<typename T>
	struct DefaultDeleter
	{
		void operator()(T* instance) const
		{
			delete instance;
		}
	};

	template<typename T, typename TDeleter = DefaultDeleter<T>>
	class Unique
	{
	public:
		Unique() = default;

		Unique(T* instance) noexcept
			: m_Instance(instance) {}

		Unique(Unique&& other) noexcept
			: m_Instance(std::exchange(other.m_Instance, nullptr))
		{
		}

		template<typename U>
		Unique(Unique<U>&& other) noexcept
			: m_Instance(static_cast<T*>(std::exchange(other.m_Instance, nullptr)))
		{
		}

		Unique& operator=(Unique&& other) noexcept
		{
			m_Instance = std::exchange(other.m_Instance, nullptr);
			return *this;
		}

		Unique& operator=(T* instance)
		{
			auto* old = std::exchange(m_Instance, instance);

			if (old)
				TDeleter()(old);

			return *this;
		}

		template<typename U>
		Unique& operator=(Unique<U>&& other) noexcept
		{
			m_Instance = static_cast<T*>(std::exchange(other.m_Instance, nullptr));
			return *this;
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
		operator TOther&() const& { return *static_cast<TOther*>(m_Instance); }

	public:
		template<typename... TArgs>
		static Unique<T> New(TArgs&&... args)
		{
			return Unique<T>(new T(std::forward<TArgs>(args)...));
		}

	private:
		T* m_Instance = nullptr;

	private:
		template<typename TOther, typename TOtherDeleter>
		friend class Unique;
	};

}
