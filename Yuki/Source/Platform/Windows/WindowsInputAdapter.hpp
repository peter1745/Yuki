#pragma once

#include "Engine/Core/Core.hpp"
#include "Engine/Core/Unique.hpp"
#include "Engine/Input/InputAdapter.hpp"

#include <GameInput.h>

namespace Yuki {

	template<>
	struct Handle<InputDevice>::Impl
	{
		InputAdapter Adapter;

		IGameInputDevice* Device = nullptr;
		const GameInputDeviceInfo* Info = nullptr;

		std::string Name;
		std::string ManufacturerName;
		std::vector<ExternalInputChannel> Channels;

		IGameInputReading* PreviousReading = nullptr;

		int64_t PreviousScrollX = 0;
		int64_t PreviousScrollY = 0;

		void WriteChannelValue(uint32_t channelIndex, float value);

		void ReadMouseInput(IGameInputReading* reading, uint32_t& currentChannel);
		void ReadKeyboardInput(IGameInputReading* reading, uint32_t& currentChannel);
		void ReadControllerAxisInput(IGameInputReading* reading, uint32_t& currentChannel);
		void ReadControllerButtonInput(IGameInputReading* reading, uint32_t& currentChannel);
		void ReadControllerSwitchInput(IGameInputReading* reading, uint32_t& currentChannel);
	};

	template<>
	struct Handle<InputAdapter>::Impl
	{
		IGameInput* Context = nullptr;

		GameInputCallbackToken DeviceCallbackToken;

		std::unordered_map<uint32_t, InputDevice> GenericDevices;
		std::unordered_map<InputDeviceID, InputDevice> Devices;

		void RegisterDevice(IGameInputDevice* device);
		void UnregisterDevice(IGameInputDevice* device);
	};

}
