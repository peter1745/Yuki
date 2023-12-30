#pragma once

#include "InputAxis.hpp"
#include "InputCodes.hpp"

namespace Yuki {

	inline constexpr uint32_t AnyDevice = ~0u;

	struct InputIDWrapper
	{
		uint32_t InputID;

		InputIDWrapper(uint32_t id)
			: InputID(id) {}

		InputIDWrapper(KeyCode code)
			: InputID(std::to_underlying(code)) {}

		InputIDWrapper(GamepadInput code)
			: InputID(std::to_underlying(code)) {}

		operator uint32_t() const { return InputID; }
	};

	struct TriggerID
	{
		uint32_t DeviceID;
		InputIDWrapper InputID;
	};

	struct TriggerBinding
	{
		TriggerID ID;
		float Scale;
	};

	struct AxisBinding
	{
		std::vector<TriggerBinding> Bindings;
	};

	struct InputAction
	{
		uint32_t ValueCount;
		std::vector<AxisBinding> AxisBindings;
		bool ConsumeInputs;
	};

}
