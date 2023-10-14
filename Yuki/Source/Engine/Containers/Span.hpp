#pragma once

#include "Engine/Common/Core.hpp"

#include <span>

namespace Yuki {

	template<typename TElementClass>
	class Span
	{
	public:
		constexpr Span(std::initializer_list<TElementClass> elements)
			: m_Span(elements) {}
		constexpr Span(const DynamicArray<TElementClass>& elements)
			: m_Span(elements) {}
		
		template<size_t Size>
		constexpr Span(const Array<TElementClass, Size>& elements)
			: m_Span(elements) {}

		constexpr bool IsEmpty() const { return m_Span.empty(); }
		constexpr size_t Count() const { return m_Span.size(); }
		constexpr size_t ByteSize() const { return m_Span.size() * sizeof(TElementClass); }

		constexpr const TElementClass& operator[](size_t index) const { return m_Span[index]; }

		constexpr auto Data() const { return m_Span.data(); }

		constexpr auto begin() const { return m_Span.begin(); }
		constexpr auto end() const { return m_Span.end(); }

		std::span<const TElementClass> ToSpan() const { return m_Span; }

	private:
		std::span<const TElementClass> m_Span;
	};

}
