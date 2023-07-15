#pragma once

#include <cmath>

namespace Yuki::Math {

	template<typename T>
	constexpr T PI() { return T(3.1415926535897932384); }

	template<typename T>
	constexpr T Radians(T InDegrees)
	{
		return InDegrees * (PI<T>() / T(180.0));
	}

	template<typename T>
	constexpr T Degrees(T InRadians)
	{
		return InRadians * (T(180.0) / PI<T>());
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
	constexpr T Atan2(T InY, T InX) { return static_cast<T>(std::atan2(static_cast<double>(InY), static_cast<double>(InX))); }

	template<typename T>
	constexpr T Asin(T InValue) { return static_cast<T>(std::asin(static_cast<double>(InValue))); }

	template<typename T>
	constexpr T Acos(T InValue) { return static_cast<T>(std::acos(static_cast<double>(InValue))); }

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
	T Sqrt(T InValue) { return T(std::sqrt(double(InValue))); }

	template<typename T>
	T Pow(T InValue, T InExponent) { return T(std::pow(double(InValue), double(InExponent))); }

	template<typename T>
	T Floor(T InValue) { return T(std::floor(double(InValue))); }

	template<typename T>
	T Abs(T InValue) { return T(std::abs(double(InValue))); }

}
