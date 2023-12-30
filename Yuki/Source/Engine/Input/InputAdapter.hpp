#pragma once

#include "InputDevice.hpp"

namespace Yuki {

	struct InputTrigger
	{
		uint32_t InputID;
		float Value = 1.0f;
	};

	struct InputBinding {};

	static constexpr uint32_t AnyKeyboardDevice = ~0u - 1;
	static constexpr uint32_t AnyGamepadDevice = ~0u - 2;

	class InputAdapter
	{
	public:
		InputAdapter();
		~InputAdapter();

		void Update();

		uint32_t GetDeviceCount() const;
		const InputDevice& GetDevice(uint32_t deviceIndex) const;
	};

}
