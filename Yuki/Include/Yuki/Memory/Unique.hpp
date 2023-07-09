#pragma once

#include <utility>

namespace Yuki {

	template<typename T>
	class Unique
	{
	public:
		constexpr Unique() noexcept = default;
		constexpr Unique(std::nullptr_t) noexcept {}

		Unique(T* InPtr) noexcept
		    : m_Ptr(InPtr) {}

		template<typename TOther>
		inline Unique(Unique<TOther>&& InOther)
		{
			m_Ptr = static_cast<T*>(InOther.m_Ptr);
			InOther.m_Ptr = nullptr;
		}

		~Unique()
		{
			Release();
		}

		void Release()
		{
			if (m_Ptr)
			{
				delete m_Ptr;
				m_Ptr = nullptr;
			}
		}

		void Reset(T* InPtr = nullptr)
		{
			Release();
			m_Ptr = InPtr;
		}

		operator T*()& { return m_Ptr; }

	public:
		template<typename... TArgs>
		static Unique<T> Create(TArgs&&... InArgs)
		{
			return Unique<T>(new T(std::forward<TArgs>(InArgs)...));
		}

	public:
		template <typename TOther>
		inline Unique& operator=(Unique<TOther>&& InOther) noexcept
		{
			Release();
			m_Ptr = static_cast<T*>(InOther.m_Ptr);
			InOther.m_Ptr = nullptr;
			return *this;
		}

		inline T& operator*() const noexcept { return *m_Ptr; }
		inline T* operator->() const noexcept { return m_Ptr; }

		//bool operator==(const Unique& InOther) const { return m_Ptr == InOther.m_Ptr; }
		//bool operator!=(const Unique& InOther) const { return !(*this == InOther); }

		operator bool() const { return m_Ptr != nullptr; }

		inline T* GetPtr() const noexcept { return m_Ptr; }

	public:
		Unique(const Unique&) = delete;
		Unique& operator=(const Unique&) = delete;

	private:
		T* m_Ptr = nullptr;

	private:
		template<typename TOther>
		friend class Unique;
	};

}
