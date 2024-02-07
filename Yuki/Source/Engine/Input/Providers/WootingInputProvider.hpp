#pragma once

#include "Engine/Input/InputDevice.hpp"

struct WootingAnalog_DeviceInfo_FFI;

namespace Yuki {

	class WootingInputProvider : public InputProvider
	{
		using WootingDeviceID = uint64_t;
	public:
		void Init(InputDeviceRegistry registry) override;
		void Update() override;

		void RegisterWootingDevice(WootingAnalog_DeviceInfo_FFI* deviceInfo);
		void UnregisterWootingDevice(WootingAnalog_DeviceInfo_FFI* deviceInfo);

	private:
		InputDeviceRegistry m_DeviceRegistry;
		std::unordered_map<InputDeviceID, WootingDeviceID> m_Devices;
	};

}
