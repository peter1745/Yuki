#pragma once

#include "../Core/Debug.hpp"

namespace Yuki {

	template<typename T, size_t ElemCount>
	class ArrayConstIterator
	{
	public:
		using PointerType = const T*;
		using ReferenceType = const T&;

	public:
		constexpr ArrayConstIterator() = default;
		constexpr explicit ArrayConstIterator(PointerType InPtr)
		    : m_Ptr(InPtr) {}

		constexpr ReferenceType operator*() const { return *m_Ptr; }
		constexpr PointerType operator->() const { return m_Ptr; }

		constexpr ArrayConstIterator& operator++()
		{
			m_Ptr++;
			return *this;
		}

		constexpr ArrayConstIterator operator++(int)
		{
			ArrayConstIterator temp = *this;
			m_Ptr++;
			return temp;
		}

		constexpr ArrayConstIterator& operator--()
		{
			m_Ptr--;
			return *this;
		}

		constexpr ArrayConstIterator operator--(int)
		{
			ArrayConstIterator temp = *this;
			m_Ptr--;
			return temp;
		}

		constexpr ArrayConstIterator& operator+=(const ptrdiff_t InOffset)
		{
			m_Ptr += InOffset;
			return *this;
		}
		
		constexpr ArrayConstIterator& operator-=(const ptrdiff_t InOffset)
		{
			m_Ptr -= InOffset;
			return *this;
		}

		constexpr ptrdiff_t operator-(const ArrayConstIterator& InOther) const
		{
			return m_Ptr - InOther.m_Ptr;
		}

		constexpr ReferenceType operator[](const ptrdiff_t InOffset) const
		{
			return m_Ptr[InOffset];
		}

		constexpr bool operator==(const ArrayConstIterator& InOther) const
		{
			return m_Ptr == InOther.m_Ptr;
		}

	private:
		PointerType m_Ptr = nullptr;
	};

	template <typename T, size_t ElemCount>
	class ArrayIterator : public ArrayConstIterator<T, ElemCount>
	{
	public:
		using Base = ArrayConstIterator<T, ElemCount>;
		using PointerType = T*;
		using ReferenceType = T&;

	public:
		constexpr ArrayIterator() = default;
		constexpr explicit ArrayIterator(PointerType InPtr)
		    : Base(InPtr) {}

		constexpr ReferenceType operator*() const { return const_cast<ReferenceType>(Base::operator*()); }
		constexpr PointerType operator->() const { return const_cast<PointerType>(Base::operator->()); }

		constexpr ArrayIterator& operator++()
		{
			Base::operator++();
			return *this;
		}

		constexpr ArrayIterator operator++(int)
		{
			ArrayIterator temp = *this;
			Base::operator++();
			return temp;
		}

		constexpr ArrayIterator& operator--()
		{
			Base::operator--();
			return *this;
		}

		constexpr ArrayIterator operator--(int)
		{
			ArrayIterator temp = *this;
			Base::operator--();
			return temp;
		}

		constexpr ArrayIterator& operator+=(const ptrdiff_t InOffset)
		{
			Base::operator+=(InOffset);
			return *this;
		}

		constexpr ArrayIterator operator+(const ptrdiff_t InOffset) const
		{
			ArrayIterator temp = *this;
			temp += InOffset;
			return temp;
		}

		friend constexpr ArrayIterator operator+(const ptrdiff_t InOffset, ArrayIterator InNext)
		{
			InNext += InOffset;
			return InNext;
		}

		constexpr ArrayIterator& operator-=(const ptrdiff_t InOffset)
		{
			Base::operator-=(InOffset);
			return *this;
		}

		using Base::operator-;

		constexpr ArrayIterator operator-(const ptrdiff_t InOffset) const
		{
			ArrayIterator temp = *this;
			temp -= InOffset;
			return temp;
		}

		constexpr ReferenceType operator[](const ptrdiff_t InOffset) const
		{
			return const_cast<ReferenceType>(Base::operator[](InOffset));
		}

	};

	template<typename T, size_t ElemCount>
	class Array
	{
	public:
		using IteratorType = ArrayIterator<T, ElemCount>;
		using ConstIteratorType = ArrayConstIterator<T, ElemCount>;
		using ReverseIteratorType = std::reverse_iterator<IteratorType>;
		using ConstReverseIteratorType = std::reverse_iterator<ConstIteratorType>;

	public:
		constexpr void Fill(const T& InValue)
		{
			for (size_t i = 0; i < ElemCount; i++)
				m_Elems[i] = InValue;
		}

		constexpr size_t Count() const { return ElemCount; }
		constexpr size_t MaxCount() const { return ElemCount; }

		constexpr bool IsEmpty() const { return false; }

		constexpr T& operator[](size_t InIndex)
		{
			YUKI_VERIFY(InIndex < ElemCount);
			return m_Elems[InIndex];
		}

		constexpr const T& operator[](size_t InIndex) const
		{
			YUKI_VERIFY(InIndex < ElemCount);
			return m_Elems[InIndex];
		}

		constexpr T* Data() { return m_Elems; }
		constexpr const T* Data() const { return m_Elems; }

		// STL Iterator Functions
		constexpr IteratorType begin() { return IteratorType(m_Elems); }
		constexpr ConstIteratorType begin() const { return ConstIteratorType(m_Elems); }

		constexpr IteratorType end() { return IteratorType(m_Elems + ElemCount); }
		constexpr ConstIteratorType end() const { return ConstIteratorType(m_Elems + ElemCount); }

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
		T m_Elems[ElemCount];
	};

}
