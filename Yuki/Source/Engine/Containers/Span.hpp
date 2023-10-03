#pragma once

#include <span>

namespace Yuki {

	template<typename TElementClass>
	class Span
	{
	public:
		constexpr Span(std::initializer_list<TElementClass> InElements)
			: m_Span(InElements) {}

		constexpr bool IsEmpty() const { return m_Span.empty(); }
		constexpr size_t Count() const { return m_Span.size(); }

		constexpr const TElementClass& operator[](size_t InIndex) const { return m_Span[InIndex]; }

		constexpr auto begin() { return m_Span.begin(); }
		constexpr auto end() { return m_Span.end(); }

	private:
		std::span<const TElementClass> m_Span;
	};

}
