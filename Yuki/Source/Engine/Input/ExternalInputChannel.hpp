#pragma once

#include "InputAxis.hpp"

namespace Yuki {

	struct ExternalInputChannel
	{
		float Value;
		float PreviousValue;

		bool IsDirty() const { return Value != PreviousValue; }
	};

}
