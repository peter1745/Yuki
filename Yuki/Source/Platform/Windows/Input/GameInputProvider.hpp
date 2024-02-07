#pragma once

#include "Engine/Input/InputDevice.hpp"

struct IGameInput;
struct IGameInputDevice;
struct IGameInputReading;

namespace Yuki {

	class GameInputProvider : public InputProvider
	{
		using CallbackToken = uint64_t;
	public:
		void Init(InputDeviceRegistry registry) override;
		void Update() override;

		void RegisterGameInputDevice(IGameInputDevice* device);
		void UnregisterGameInputDevice(IGameInputDevice* device);

	private:
		void ReadMouseInput(InputDeviceID deviceID, IGameInputReading* reading, uint32_t& currentChannel);
		void ReadKeyboardInput(InputDeviceID deviceID, IGameInputReading* reading, uint32_t& currentChannel);
		void ReadControllerAxisInput(InputDeviceID deviceID, IGameInputReading* reading, uint32_t& currentChannel);
		void ReadControllerButtonInput(InputDeviceID deviceID, IGameInputReading* reading, uint32_t& currentChannel);
		void ReadControllerSwitchInput(InputDeviceID deviceID, IGameInputReading* reading, uint32_t& currentChannel);

	private:
		IGameInput* m_GameInput = nullptr;

		CallbackToken m_DeviceCallbackToken = 0;

		std::unordered_map<InputDeviceID, IGameInputDevice*> m_Devices;

		InputDeviceRegistry m_DeviceRegistry;
	};

}
