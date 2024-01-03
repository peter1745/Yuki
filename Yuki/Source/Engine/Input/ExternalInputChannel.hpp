#pragma once

#include "Engine/Core/Core.hpp"

#include "InputAxis.hpp"

namespace Yuki {

	struct ExternalInputChannel
	{
		float32_t Value;
		float32_t PreviousValue;

		bool HasRegisteredInput() const { return Value != 0.0f || PreviousValue != 0.0f; }
	};

}
