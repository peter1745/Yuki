#pragma once

#include <cmath>

namespace Yuki::Math {

	template<typename T>
	constexpr T PI() { return T(3.1415926535897932384); }

	template<typename T>
	constexpr T Radians(T InValue)
	{
		return InValue * T(PI<T>() / 180.0);
	}

	template<typename T>
	constexpr T Cos(T InValue) { return T(std::cos(double(InValue))); }

	template<typename T>
	constexpr T Sin(T InValue) { return T(std::sin(double(InValue))); }

	template<typename T>
	constexpr T Min(T InValue, T InOtherValue) { return InValue < InOtherValue ? InValue : InOtherValue; }

	template<typename T>
	constexpr T Max(T InValue, T InOtherValue) { return InValue > InOtherValue ? InValue : InOtherValue; }

	template<typename T>
	constexpr T Clamp(T InValue, T InMin, T InMax)
	{
		if (InValue > InMax)
			return InMax;

		if (InValue < InMin)
			return InMin;

		return InValue;
	}

	template<typename T>
	constexpr T Sqrt(T InValue) { return T(std::sqrt(double(InValue))); }

}
