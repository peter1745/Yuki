#pragma once

#include <array>
#include <vector>
#include <functional>
#include <map>
#include <unordered_map>
#include <initializer_list>

namespace Yuki {

	template<typename TElementClass, size_t Size>
	using Array = std::array<TElementClass, Size>;

	template<typename TElementClass>
	using DynamicArray = std::vector<TElementClass>;

	template<typename TKeyClass, typename TElementClass>
	using Map = std::map<TKeyClass, TElementClass>;

	template<typename TKeyClass, typename TElementClass>
	using HashMap = std::unordered_map<TKeyClass, TElementClass>;

	template<typename TFunction>
	using Function = std::function<TFunction>;

}
