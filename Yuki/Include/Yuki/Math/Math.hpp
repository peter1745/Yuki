#pragma once

namespace Yuki {

	class Math
	{
	public:
		template<typename T>
		static constexpr T Min(T InValue, T InOtherValue) { return InValue < InOtherValue ? InValue : InOtherValue; }

		template<typename T>
		static constexpr T Clamp(T InValue, T InMin, T InMax)
		{
			if (InValue > InMax)
				return InMax;

			if (InValue < InMin)
				return InMin;

			return InValue;
		}

	};

}
