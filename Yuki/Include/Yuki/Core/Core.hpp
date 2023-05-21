#pragma once

#include <array>
#include <vector>
#include <ankerl/unordered_dense.h>

namespace Yuki {

	template<typename T, size_t Size>
	using Array = std::array<T, Size>;

	template<typename T>
	using List = std::vector<T>;

	template<typename TKey, typename TValue, typename THash = ankerl::unordered_dense::v4_0_0::hash<TKey>>
	using Map = ankerl::unordered_dense::map<TKey, TValue, THash>;

}
