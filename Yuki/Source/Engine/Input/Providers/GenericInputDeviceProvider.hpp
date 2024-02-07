#pragma once

#include "Engine/Input/InputDevice.hpp"

namespace Yuki {

	class GenericInputDeviceProvider : public InputProvider
	{
	public:
		void Init(InputDeviceRegistry registry) override;
		void Update() override;

	private:
		InputDeviceRegistry m_DeviceRegistry;
	};

}
