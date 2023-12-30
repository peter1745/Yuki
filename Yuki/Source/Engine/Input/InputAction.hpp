#pragma once

#include "InputAxis.hpp"

namespace Yuki {

	inline constexpr uint32_t AnyDevice = ~0u;

	struct TriggerID
	{
		uint32_t DeviceID;
		uint32_t InputID;
	};

	struct TriggerBinding
	{
		TriggerID ID;
		float Scale;
	};

	struct AxisBinding
	{
		Axis TargetAxis;
		std::vector<TriggerBinding> Bindings;
	};

	struct InputAction
	{
		AxisType Type;
		std::vector<AxisBinding> AxisBindings;
		bool ConsumeInputs;
	};

}
