#pragma once

#include "InputDevice.hpp"

namespace Yuki {

	inline constexpr uint32_t s_MaxKeyCount = 256;

	// Max supported mouse buttons according to GameInput + scroll wheel x and y
	inline constexpr uint32_t s_MaxMouseButtons = 7;
	inline constexpr uint32_t s_MaxMouseScrollAxes = 2;
	inline constexpr uint32_t s_MaxMouseChannels = s_MaxMouseButtons + s_MaxMouseScrollAxes;
	inline constexpr uint32_t s_MaxGamepadChannels = 48;

	inline constexpr uint32_t s_MaxControllerAxes = 256;
	inline constexpr uint32_t s_MaxControllerButtons = 512;
	inline constexpr uint32_t s_MaxControllerSwitches = 512;

	template<>
	struct Handle<InputDevice>::Impl
	{
		InputDevice::Type Type = InputDevice::Type::Unknown;

		std::string Name;
		std::string ManufacturerName;
		std::vector<ExternalInputChannel> Channels;

		bool HasScrollX = false;
		bool HasScrollY = false;

		int64_t PreviousScrollX = 0;
		int64_t PreviousScrollY = 0;

		void WriteChannelValue(uint32_t channelIndex, float32_t value);
	};

	template<>
	struct Handle<InputDeviceRegistry>::Impl
	{
		std::unordered_map<InputDeviceID, InputDevice> Devices;

		InputDevice CreateDevice(InputDeviceID deviceID);
	};

}
