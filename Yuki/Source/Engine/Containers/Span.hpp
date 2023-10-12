#pragma once

#include <span>

namespace Yuki {

	template<typename TElementClass>
	class Span
	{
	public:
		constexpr Span(std::initializer_list<TElementClass> elements)
			: m_Span(elements) {}

		constexpr bool IsEmpty() const { return m_Span.empty(); }
		constexpr size_t Count() const { return m_Span.size(); }

		constexpr const TElementClass& operator[](size_t index) const { return m_Span[index]; }

		constexpr auto begin() const { return m_Span.begin(); }
		constexpr auto end() const { return m_Span.end(); }

		std::span<const TElementClass> ToSpan() const { return m_Span; }

	private:
		std::span<const TElementClass> m_Span;
	};

}
