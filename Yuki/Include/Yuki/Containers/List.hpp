#pragma once

#include <limits>

namespace Yuki {

	template <typename T>
	class ListConstIterator
	{
	public:
		using PointerType = const T*;
		using ReferenceType = const T&;

	public:
		constexpr ListConstIterator() = default;
		constexpr explicit ListConstIterator(PointerType InPtr)
		    : m_Ptr(InPtr) {}

		constexpr ReferenceType operator*() const { return *m_Ptr; }
		constexpr PointerType operator->() const { return m_Ptr; }

		constexpr ListConstIterator& operator++()
		{
			m_Ptr++;
			return *this;
		}

		constexpr ListConstIterator operator++(int)
		{
			ListConstIterator temp = *this;
			m_Ptr++;
			return temp;
		}

		constexpr ListConstIterator& operator--()
		{
			m_Ptr--;
			return *this;
		}

		constexpr ListConstIterator operator--(int)
		{
			ListConstIterator temp = *this;
			m_Ptr--;
			return temp;
		}

		constexpr ListConstIterator& operator+=(const ptrdiff_t InOffset)
		{
			m_Ptr += InOffset;
			return *this;
		}

		constexpr ListConstIterator& operator-=(const ptrdiff_t InOffset)
		{
			m_Ptr -= InOffset;
			return *this;
		}

		constexpr ptrdiff_t operator-(const ListConstIterator& InOther) const
		{
			return m_Ptr - InOther.m_Ptr;
		}

		constexpr ReferenceType operator[](const ptrdiff_t InOffset) const
		{
			return m_Ptr[InOffset];
		}

		constexpr bool operator==(const ListConstIterator& InOther) const
		{
			return m_Ptr == InOther.m_Ptr;
		}

	private:
		PointerType m_Ptr = nullptr;
	};

	template <typename T>
	class ListIterator : public ListConstIterator<T>
	{
	public:
		using Base = ListConstIterator<T>;
		using PointerType = T*;
		using ReferenceType = T&;

	public:
		constexpr ListIterator() = default;
		constexpr explicit ListIterator(PointerType InPtr)
		    : Base(InPtr) {}

		constexpr ReferenceType operator*() const { return const_cast<ReferenceType>(Base::operator*()); }
		constexpr PointerType operator->() const { return const_cast<PointerType>(Base::operator->()); }

		constexpr ListIterator& operator++()
		{
			Base::operator++();
			return *this;
		}

		constexpr ListIterator operator++(int)
		{
			ListIterator temp = *this;
			Base::operator++();
			return temp;
		}

		constexpr ListIterator& operator--()
		{
			Base::operator--();
			return *this;
		}

		constexpr ListIterator operator--(int)
		{
			ListIterator temp = *this;
			Base::operator--();
			return temp;
		}

		constexpr ListIterator& operator+=(const ptrdiff_t InOffset)
		{
			Base::operator+=(InOffset);
			return *this;
		}

		constexpr ListIterator operator+(const ptrdiff_t InOffset) const
		{
			ListIterator temp = *this;
			temp += InOffset;
			return temp;
		}

		friend constexpr ListIterator operator+(const ptrdiff_t InOffset, ListIterator InNext)
		{
			InNext += InOffset;
			return InNext;
		}

		constexpr ListIterator& operator-=(const ptrdiff_t InOffset)
		{
			Base::operator-=(InOffset);
			return *this;
		}

		using Base::operator-;

		constexpr ListIterator operator-(const ptrdiff_t InOffset) const
		{
			ListIterator temp = *this;
			temp -= InOffset;
			return temp;
		}

		constexpr ReferenceType operator[](const ptrdiff_t InOffset) const
		{
			return const_cast<ReferenceType>(Base::operator[](InOffset));
		}
	};

	template<typename T>
	class List
	{
	public:
		using IteratorType = ListIterator<T>;
		using ConstIteratorType = ListConstIterator<T>;
		using ReverseIteratorType = std::reverse_iterator<IteratorType>;
		using ConstReverseIteratorType = std::reverse_iterator<ConstIteratorType>;

	public:
		constexpr List() = default;

		constexpr List(size_t InElementCount, const T& InValue = T())
		    : m_Count(InElementCount), m_Capacity(InElementCount)
		{
			m_Data = reinterpret_cast<T*>(::operator new(m_Capacity * sizeof(T)));
			for (size_t i = 0; i < m_Count; i++)
				m_Data[i] = std::move(InValue);
		}

		constexpr List(const List& InOther)
		    : m_Count(InOther.m_Count), m_Capacity(InOther.m_Capacity)
		{
			m_Data = reinterpret_cast<T*>(::operator new(m_Capacity * sizeof(T)));
			memmove(m_Data, InOther.m_Data, m_Count * sizeof(T));
		}

		constexpr List(List&& InOther) noexcept
		    : m_Data(std::move(InOther.m_Data)), m_Count(InOther.m_Count), m_Capacity(InOther.m_Capacity)
		{
		}

		constexpr ~List()
		{
			Deallocate();
		}

		constexpr List& operator=(const List& InOther)
		{
			Deallocate();

			m_Data = reinterpret_cast<T*>(::operator new(m_Capacity * sizeof(T)));
			memmove(m_Data, InOther.m_Data, m_Count * sizeof(T));
			return *this;
		}

		constexpr List& operator=(List&& InOther) noexcept
		{
			Deallocate();

			m_Data = std::move(InOther.m_Data);
			m_Count = InOther.m_Count;
			m_Capacity = InOther.m_Capacity;
			return *this;
		}

		constexpr bool IsEmpty() const { return m_Count == 0; }
		
		constexpr size_t Count() const { return m_Count; }
		constexpr size_t MaxCount() const { return UINT64_MAX; }

		constexpr size_t Capacity() const { return m_Capacity; }

		constexpr T* Data() { return m_Data; }
		constexpr const T* Data() const { return m_Data; }

		constexpr void Clear()
		{
			if (m_Count == 0)
				return;

			for (size_t i = 0; i < m_Count; i++)
				m_Data[i].~T();

			m_Count = 0;
		}

		template<typename... TArgs>
		constexpr T& EmplaceBack(TArgs&&... InArgs)
		{
			if (m_Count < m_Capacity)
			{
				return EmplaceWithUnusedCapacity(std::forward<TArgs>(InArgs)...);
			}

			return EmplaceReallocate(std::forward<TArgs>(InArgs)...);
		}

		constexpr void Resize(size_t InCount)
		{
			size_t newCapacity = CalculateGrowth(InCount);
			size_t elemsToCopy = newCapacity < m_Count ? newCapacity : m_Count;

			auto* newData = reinterpret_cast<T*>(::operator new(newCapacity * sizeof(T)));
			memset(newData, 0, newCapacity * sizeof(T));

			for (size_t i = 0; i < elemsToCopy; i++)
				newData[i] = std::move(m_Data[i]);

			Deallocate();
			m_Data = newData;
			m_Count = InCount;
			m_Capacity = newCapacity;
		}

		constexpr T& operator[](size_t InIndex) { return m_Data[InIndex]; }
		constexpr const T& operator[](size_t InIndex) const { return m_Data[InIndex]; }

	public:
		// STL Iterator Functions
		constexpr IteratorType begin() { return IteratorType(m_Data); }
		constexpr ConstIteratorType begin() const { return ConstIteratorType(m_Data); }

		constexpr IteratorType end() { return IteratorType(m_Data + m_Count); }
		constexpr ConstIteratorType end() const { return ConstIteratorType(m_Data + m_Count); }

		constexpr ReverseIteratorType rbegin() { return ReverseIteratorType(end()); }
		constexpr ConstReverseIteratorType rbegin() const { return ConstReverseIteratorType(end()); }

		constexpr ReverseIteratorType rend() { return ReverseIteratorType(begin()); }
		constexpr ConstReverseIteratorType rend() const { return ConstReverseIteratorType(begin()); }

		constexpr ConstIteratorType cbegin() { return begin(); }
		constexpr ConstIteratorType cend() const { return end(); }

		constexpr ConstReverseIteratorType crbegin() { return rbegin(); }
		constexpr ConstReverseIteratorType crend() const { return rend(); }

		// Custom Iterator Functions (Mainly for consistency)
		constexpr IteratorType Begin() { return begin(); }
		constexpr ConstIteratorType Begin() const { return begin(); }

		constexpr IteratorType End() { return end(); }
		constexpr ConstIteratorType End() const { return end(); }

	private:
		template<typename... TArgs>
		constexpr T& EmplaceWithUnusedCapacity(TArgs&&... InArgs)
		{
			auto* result = new (&m_Data[m_Count]) T(std::forward<TArgs>(InArgs)...);
			m_Count++;
			return *result;
		}

		template<typename... TArgs>
		constexpr T& EmplaceReallocate(TArgs&&... InArgs)
		{
			size_t newCount = m_Count + 1;
			size_t newCapacity = CalculateGrowth(newCount);

			auto* newData = reinterpret_cast<T*>(::operator new(newCapacity * sizeof(T)));

			for (size_t i = 0; i < m_Count; i++)
				newData[i] = std::move(m_Data[i]);

			auto* result = new (&newData[m_Count]) T(std::forward<TArgs>(InArgs)...);

			// Clean up old array
			Deallocate();
			m_Data = newData;
			m_Count = newCount;
			m_Capacity = newCapacity;

			return *result;
		}

		constexpr void Deallocate()
		{
			if (!m_Data)
				return;

			for (size_t i = 0; i < m_Count; i++)
				m_Data[i].~T();
			::operator delete(m_Data, m_Capacity * sizeof(T));
		}

		constexpr size_t CalculateGrowth(size_t InNewSize) const
		{
			size_t oldCapacity = m_Capacity;
			size_t maxCapacity = MaxCount();

			if (oldCapacity > maxCapacity - oldCapacity / 2)
				return maxCapacity;

			size_t geometric = oldCapacity + oldCapacity / 2;

			if (geometric < InNewSize)
				return InNewSize;

			return geometric;
		}

	private:
		T* m_Data = nullptr;
		size_t m_Count = 0;
		size_t m_Capacity = 0;
	};

}
