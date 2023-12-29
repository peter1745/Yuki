#pragma once

#include <variant>

namespace Yuki {

	enum class Axis
	{
		None,
		X,
		Y
	};

	struct AxisValue1D { float Value; };
	struct AxisValue2D { float X; float Y; };
	struct AxisValue3D { float X; float Y; float Z; };

	template<typename T>
	concept AxisValueType = std::same_as<T, AxisValue1D> || std::same_as<T, AxisValue2D> || std::same_as<T, AxisValue3D>;

	using AxisValue = std::variant<AxisValue1D, AxisValue2D, AxisValue3D>;

}
