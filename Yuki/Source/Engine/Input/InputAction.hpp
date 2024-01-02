#pragma once

#include "Engine/Core/Core.hpp"
#include "Engine/Core/Handle.hpp"

#include "InputAxis.hpp"
#include "InputCodes.hpp"

#include <vector>

namespace Yuki {

	inline constexpr uint32_t AnyDevice = ~0u;

	struct InputIDWrapper
	{
		uint32_t InputID;

		template<CastableTo<uint32_t> T>
		InputIDWrapper(T value)
			: InputID(static_cast<uint32_t>(value))
		{
		}

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
		float32_t Scale;
	};

	struct AxisBinding
	{
		std::vector<TriggerBinding> Bindings;
	};

	struct InputActionData
	{
		uint32_t ValueCount;
		std::vector<AxisBinding> AxisBindings;
		bool ConsumeInputs;
	};

	struct InputAction : Handle<InputAction>
	{
		void AddTrigger(uint32_t axis, const TriggerBinding& triggerBinding);
		void RemoveTrigger(uint32_t axis, uint32_t trigger);
		void ReplaceTrigger(uint32_t axis, uint32_t trigger, TriggerID triggerID);
	};

}
