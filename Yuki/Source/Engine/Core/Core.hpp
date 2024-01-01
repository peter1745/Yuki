#pragma once

namespace Yuki {

	template<typename From, typename To>
	concept CastableTo = requires { static_cast<To>(std::declval<From>()); };

	using float32_t = float;
	using float64_t = double;

	constexpr float32_t operator""_f32(long double value)
	{
		return static_cast<float32_t>(value);
	}

	constexpr float64_t operator""_f64(long double value)
	{
		return static_cast<float64_t>(value);
	}
}

#define YukiUnused(x) (void)x
