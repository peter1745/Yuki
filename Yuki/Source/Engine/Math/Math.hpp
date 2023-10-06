#pragma once

#include <concepts>

namespace Yuki::Math {

	template<typename T>
	concept Numerical = std::floating_point<T> || std::integral<T> || std::unsigned_integral<T>;

	using FPType = float;

}
