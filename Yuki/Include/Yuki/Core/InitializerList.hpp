#pragma once

#include <initializer_list>

namespace Yuki {

	template <typename T>
	class InitializerList
	{
	public:
		std::initializer_list<T> List;

		constexpr InitializerList(const std::initializer_list<T>& InList)
		    : List(InList)
		{
		}

		constexpr InitializerList(const T* InBegin, const T* InEnd)
		    : List({ InBegin, InEnd })
		{
		}

		constexpr bool Empty() const { return List.size() == 0; }

		constexpr size_t Size() const { return List.size(); }

		constexpr T operator[](size_t InIndex) const { return *(List.begin() + InIndex); }
	};

}
