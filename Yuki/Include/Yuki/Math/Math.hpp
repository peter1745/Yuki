#pragma once

namespace Yuki::Math {

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
