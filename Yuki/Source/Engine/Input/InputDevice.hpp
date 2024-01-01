#pragma once

#include "Engine/Core/Exception.hpp"
#include "Engine/Core/Handle.hpp"

#include "ExternalInputChannel.hpp"

namespace Yuki {

	inline constexpr uint32_t GenericMouse    = ~0u - 1;
	inline constexpr uint32_t GenericKeyboard = ~0u - 2;
	inline constexpr uint32_t GenericGamepad  = ~0u - 3;

	using InputDeviceID = uint32_t;

	struct InputDevice : Handle<InputDevice>
	{
		enum class Type
		{
			Unknown,
			Mouse,
			Keyboard,
			Gamepad,
			Controller
		};

		std::string_view GetName() const;
		std::string_view GetManufacturerName() const;
		Type GetType() const;

		const ExternalInputChannel* GetChannel(uint32_t channelIndex) const;
	};

}
