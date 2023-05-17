#pragma once

#include <array>
#include <vector>

namespace Yuki {

	template<typename T, size_t Size>
	using Array = std::array<T, Size>;

	template<typename T>
	using List = std::vector<T>;

}
