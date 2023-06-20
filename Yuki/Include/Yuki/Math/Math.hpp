#pragma once

#include <cmath>

namespace Yuki::Math {

	template<typename T>
	constexpr T PI() { return T(3.1415926535897932384); }

	template<typename T>
	constexpr T Radians(T InValue)
	{
		return InValue * T(180.0 / PI<T>());
	}

	template<typename T>
	constexpr T Min(T InValue, T InOtherValue) { return InValue < InOtherValue ? InValue : InOtherValue; }

	template<typename T>
	constexpr T Clamp(T InValue, T InMin, T InMax)
	{
		if (InValue > InMax)
			return InMax;

		if (InValue < InMin)
			return InMin;

		return InValue;
	}

}
