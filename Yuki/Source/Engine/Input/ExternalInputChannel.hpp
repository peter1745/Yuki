#pragma once

#include "Engine/Core/Core.hpp"

#include "InputAxis.hpp"

namespace Yuki {

	struct ExternalInputChannel
	{
		float32_t Value;
		float32_t PreviousValue;

		bool IsDirty() const { return Value != PreviousValue; }
	};

}
