#pragma once

#include <initializer_list>

namespace Yuki {

	template <typename T>
	class InitializerList
	{
	public:
		constexpr InitializerList(const std::initializer_list<T>& InList)
		    : m_List(InList)
		{
		}

		constexpr bool Empty() const { return m_List.size() == 0; }

		constexpr size_t Size() const { return m_List.size(); }

		constexpr T operator[](size_t InIndex) const { return *(m_List.begin() + InIndex); }

	private:
		std::initializer_list<T> m_List;
	};

}
