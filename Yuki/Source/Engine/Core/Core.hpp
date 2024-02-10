#pragma once

#include "Logging.hpp"

#include <stacktrace>
#include <random>

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


namespace Yuki {

	template<typename From, typename To>
	concept CastableTo = requires { static_cast<To>(std::declval<From>()); };

	template<typename Func, typename Return, typename... Args>
	concept SameReturn = requires(Func func, Args&&... args)
	{
		{ func(std::forward<Args>(args)...) } -> std::same_as<Return>;
	};

	template<typename Container>
	concept ResizableContainer = requires(Container c)
	{
		{ c.resize(1ULL) };
	}
	|| requires(Container c)
	{
		{ c.Resize(1ULL) };
	};

	template<typename Container>
	concept ContiguousContainer = std::ranges::contiguous_range<Container>;

	template<typename T>
	T RandomRange(T from, T to)
	{
		std::random_device rd;
		std::mt19937_64 generator(rd());
		std::uniform_int_distribution<T> distribution(from, to);
		return distribution(generator);
	}
}

#define YukiUnused(x) (void)x

#if defined(YUKI_PLATFORM_WINDOWS)
	#define YukiDebugBreak __debugbreak
#else
	#error Yuki only supports building on Windows
#endif

#define YukiAssert(Expr)                                   \
do {                                                       \
	if (!(Expr))                                           \
	{                                                      \
		auto st = ::std::stacktrace::current();            \
		::Yuki::WriteLine("Expression {} failed.", #Expr); \
		YukiDebugBreak();                                  \
	}                                                      \
} while (false)
