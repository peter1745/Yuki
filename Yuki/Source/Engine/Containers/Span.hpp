#pragma once

namespace Yuki {

	template<typename T>
	class Span
	{
	public:
		using ValueType = T;
		using SizeType = uint64_t;

	public:
		constexpr Span() = default;

		constexpr Span(ValueType* first, SizeType count)
			: m_Ptr(first), m_Count(count) {}

		constexpr Span(ValueType* first, ValueType* last)
		{
			m_Ptr = first;
			m_Count = std::distance(first, last);
		}

		constexpr Span(std::initializer_list<ValueType> list)
			: m_Ptr(const_cast<ValueType*>(list.begin())), m_Count(static_cast<SizeType>(list.size()))
		{
		}

		constexpr SizeType Count() const { return m_Count; }
		constexpr SizeType ByteCount() const { return m_Count * sizeof(ValueType); }

		constexpr ValueType& operator[](SizeType index)
		{
			//AuraVerify(index < self.m_Count, "Index out of range!");
			return m_Ptr[index];
		}

		constexpr const ValueType& operator[](SizeType index) const
		{
			//AuraVerify(index < self.m_Count, "Index out of range!");
			return m_Ptr[index];
		}

		constexpr ValueType* Begin() { return m_Ptr; }
		constexpr const ValueType* Begin() const { return m_Ptr; }

		constexpr ValueType* End() { return m_Ptr + m_Count; }
		constexpr const ValueType* End() const { return m_Ptr + m_Count; }

	private:
		ValueType* m_Ptr = nullptr;
		SizeType m_Count = 0;
	};

	template<typename T>
	constexpr T* begin(Span<T>& t) noexcept { return t.Begin(); }

	template<typename T>
	constexpr const T* begin(const Span<T>& t) noexcept { return t.Begin(); }

	template<typename T>
	constexpr T* end(Span<T>& t) noexcept { return t.End(); }

	template<typename T>
	constexpr const T* end(const Span<T>& t) noexcept { return t.End(); }

}
