#include "Engine/Input/InputDisptacher.hpp"

#include "WindowsCommon.hpp"

#include <GameInput.h>

#include <initguid.h>
#include <cfgmgr32.h>
#include <devpkey.h>
#include <hidsdi.h>

#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "Hid")

namespace Yuki {

	static IGameInput* s_GameInput;
	static GameInputCallbackToken s_DeviceCallbackToken;
	static std::vector<InputDevice> s_InputDevices;

	// Ugly mess to deal with the fact that GameInput doesn't provide us with a user friendly name currently
	std::pair<std::string, std::string> FetchDeviceNames(IGameInputDevice* device)
	{
		const auto* deviceInfo = device->GetDeviceInfo();

		if (deviceInfo->deviceFamily == GameInputFamilyXboxOne)
		{
			return { "Xbox One Controller", "Microsoft" };
		}
		else if (deviceInfo->deviceFamily == GameInputFamilyXbox360)
		{
			return { "Xbox 360 Controller", "Microsoft" };
		}

		if (deviceInfo->supportedInput == GameInputKindMouse)
		{
			return { "Generic Mouse", "Unknown Manufacturer" };
		}
		else if (deviceInfo->supportedInput == GameInputKindKeyboard)
		{
			return { "Generic Keyboard", "Unknown Manufacturer" };
		}

		uint32_t rawDeviceCount;
		GetRawInputDeviceList(nullptr, &rawDeviceCount, sizeof(RAWINPUTDEVICELIST));
		std::vector<RAWINPUTDEVICELIST> rawDevices(rawDeviceCount);
		GetRawInputDeviceList(rawDevices.data(), &rawDeviceCount, sizeof(RAWINPUTDEVICELIST));

		for (auto rawDevice : rawDevices)
		{
			// Try and use the data in RID_DEVICE_INFO to associate `device` with a device from s_Devices
			uint32_t ridDeviceInfoSize = sizeof(RID_DEVICE_INFO);
			RID_DEVICE_INFO ridDeviceInfo{};
			if (GetRawInputDeviceInfo(rawDevice.hDevice, RIDI_DEVICEINFO, &ridDeviceInfo, &ridDeviceInfoSize) == ~0u)
			{
				std::cout << "Skipping device because we failed to get the necessary info to associate it with a GameInput device.\n";
				continue;
			}

			uint32_t devicePathSize;
			GetRawInputDeviceInfo(rawDevice.hDevice, RIDI_DEVICENAME, nullptr, &devicePathSize);
			std::wstring devicePath(devicePathSize, 0);
			GetRawInputDeviceInfo(rawDevice.hDevice, RIDI_DEVICENAME, devicePath.data(), &devicePathSize);

			std::wstring friendlyName;
			std::wstring manufacturerName;

			if (deviceInfo->vendorId != ridDeviceInfo.hid.dwVendorId || deviceInfo->productId != ridDeviceInfo.hid.dwProductId)
			{
				continue;
			}

			DEVPROPTYPE propertyType;
			ULONG propertySize = 0;
			CONFIGRET cr = CM_Get_Device_Interface_PropertyW(devicePath.c_str(), &DEVPKEY_Device_InstanceId, &propertyType, nullptr, &propertySize, 0);

			if (cr != CR_BUFFER_SMALL)
			{
				__debugbreak();
			}

			std::wstring deviceId;
			deviceId.resize(propertySize);
			cr = CM_Get_Device_Interface_PropertyW(devicePath.c_str(), &DEVPKEY_Device_InstanceId, &propertyType, reinterpret_cast<PBYTE>(deviceId.data()), &propertySize, 0);

			if (cr != CR_SUCCESS)
			{
				__debugbreak();
			}

			DEVINST devInst;
			cr = CM_Locate_DevNodeW(&devInst, reinterpret_cast<DEVINSTID>(deviceId.data()), CM_LOCATE_DEVNODE_NORMAL);

			if (cr != CR_SUCCESS)
			{
				__debugbreak();
			}

			propertySize = 0;
			cr = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_NAME, &propertyType, nullptr, &propertySize, 0);
			if (cr != CR_BUFFER_SMALL)
			{
				__debugbreak();
			}

			friendlyName.resize(propertySize);
			cr = ::CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_NAME, &propertyType, reinterpret_cast<PBYTE>(friendlyName.data()), &propertySize, 0);

			propertySize = 0;
			cr = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_Manufacturer, &propertyType, nullptr, &propertySize, 0);
			if (cr != CR_BUFFER_SMALL)
			{
				__debugbreak();
			}

			manufacturerName.resize(propertySize);
			cr = ::CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_Manufacturer, &propertyType, reinterpret_cast<PBYTE>(manufacturerName.data()), &propertySize, 0);
			
			return { Utf16ToUtf8(friendlyName), Utf16ToUtf8(manufacturerName) };
		}

		return { "Generic HID Device", "Unknown Manufacturer" };
	}

	/// <summary>
	/// Calculates the number of external channels that we need
	/// to represent all possible inputs on a given device
	/// </summary>
	/// <param name="deviceInfo">The GameInputDeviceInfo struct for the device we're checking.</param>
	/// <returns>The number of external channels in a device</returns>
	uint32_t GetRequiredChannelCount(const GameInputDeviceInfo* deviceInfo)
	{
		uint32_t channelCount = 0;

		if (deviceInfo->mouseInfo)
		{
			channelCount += std::popcount(static_cast<uint32_t>(deviceInfo->mouseInfo->supportedButtons));
			channelCount += deviceInfo->mouseInfo->hasWheelX;
			channelCount += deviceInfo->mouseInfo->hasWheelY;
		}

		if (deviceInfo->keyboardInfo)
		{
			channelCount += deviceInfo->keyboardInfo->keyCount;
			// Is deviceInfo->keyboardInfo->functionKeyCount accounted for in deviceInfo->keyboardInfo->keyCount?
		}

		channelCount += deviceInfo->controllerAxisCount;
		channelCount += deviceInfo->controllerButtonCount;

		// A switch can be represented as a 2-axis input (x and y)
		channelCount += deviceInfo->controllerSwitchCount * 2;

		return channelCount;
	}

	void CALLBACK DeviceCallback(
		_In_ GameInputCallbackToken callbackToken,
		_In_ void* context,
		_In_ IGameInputDevice* device,
		_In_ uint64_t timestamp,
		_In_ GameInputDeviceStatus currentStatus,
		_In_ GameInputDeviceStatus previousStatus)
	{
		if (!(currentStatus & GameInputDeviceConnected))
			return;

		const auto* deviceInfo = device->GetDeviceInfo();

		auto[deviceName, manufacturer] = FetchDeviceNames(device);
		uint32_t requiredChannels = GetRequiredChannelCount(deviceInfo);

		InputDevice::Type deviceType = InputDevice::Type::Unknown;

		if (deviceInfo->supportedInput & GameInputKindMouse)
		{
			deviceType = InputDevice::Type::Mouse;
		}
		else if (deviceInfo->supportedInput & GameInputKindKeyboard)
		{
			deviceType = InputDevice::Type::Keyboard;
		}
		else if (deviceInfo->supportedInput & GameInputKindController)
		{
			deviceType = InputDevice::Type::Controller;
		}

		s_InputDevices.emplace_back(deviceType, deviceName, manufacturer, requiredChannels, device);

		std::cout << "Device: " << manufacturer << " " << deviceName << "\n";
		if (deviceInfo->supportedInput & GameInputKindUnknown)
			std::cout << "\tUnknown Input Kind\n";
		if (deviceInfo->supportedInput & GameInputKindRawDeviceReport)
			std::cout << "\tRaw Device Report\n";
		if (deviceInfo->supportedInput & GameInputKindControllerAxis)
			std::cout << "\tController Axis\n";
		if (deviceInfo->supportedInput & GameInputKindControllerButton)
			std::cout << "\tController Button\n";
		if (deviceInfo->supportedInput & GameInputKindControllerSwitch)
			std::cout << "\tController Switch\n";
		if (deviceInfo->supportedInput & GameInputKindController)
			std::cout << "\tController\n";
		if (deviceInfo->supportedInput & GameInputKindKeyboard)
			std::cout << "\tKeyboard\n";
		if (deviceInfo->supportedInput & GameInputKindMouse)
			std::cout << "\tMouse\n";
		if (deviceInfo->supportedInput & GameInputKindTouch)
			std::cout << "\tTouch\n";
		if (deviceInfo->supportedInput & GameInputKindMotion)
			std::cout << "\tMotion\n";
		if (deviceInfo->supportedInput & GameInputKindArcadeStick)
			std::cout << "\tArcade Stick\n";
		if (deviceInfo->supportedInput & GameInputKindFlightStick)
			std::cout << "\tFlight Stick\n";
		if (deviceInfo->supportedInput & GameInputKindGamepad)
			std::cout << "\tGamepad\n";
		if (deviceInfo->supportedInput & GameInputKindRacingWheel)
			std::cout << "\tRacing Wheel\n";
		if (deviceInfo->supportedInput & GameInputKindUiNavigation)
			std::cout << "\tUI Navigation\n";

		std::cout << "Required Channels: " << requiredChannels << "\n";
	}

	InputDispatcher::InputDispatcher()
	{
		CheckHR(GameInputCreate(&s_GameInput));

		CheckHR(s_GameInput->RegisterDeviceCallback(
			nullptr,
			GameInputKindController |
			GameInputKindKeyboard |
			GameInputKindMouse |
			GameInputKindGamepad |
			GameInputKindFlightStick |
			GameInputKindArcadeStick |
			GameInputKindRacingWheel,
			GameInputDeviceAnyStatus,
			GameInputBlockingEnumeration,
			this,
			DeviceCallback,
			&s_DeviceCallbackToken
		));

		s_GameInput->SetFocusPolicy(GameInputDefaultFocusPolicy);
	}

	InputDispatcher::~InputDispatcher()
	{
		s_GameInput->UnregisterCallback(s_DeviceCallbackToken, 0);
	}

	float SwitchPositionToAxisValue(GameInputSwitchPosition position, Axis axis)
	{
		if (position == GameInputSwitchCenter)
		{
			return 0.0f;
		}

		if (axis == Axis::X)
		{
			switch (position)
			{
			case GameInputSwitchUpRight: return 0.5f;
			case GameInputSwitchRight: return 1.0f;
			case GameInputSwitchDownRight: return 0.5f;
			case GameInputSwitchDownLeft: return -0.5f;
			case GameInputSwitchLeft: return -1.0f;
			case GameInputSwitchUpLeft: return -0.5f;
			}
		}
		else if (axis == Axis::Y)
		{
			switch (position)
			{
			case GameInputSwitchUp: return 1.0f;
			case GameInputSwitchUpRight: return 0.5f;
			case GameInputSwitchDown: return -1.0f;
			case GameInputSwitchDownRight: return -0.5f;
			case GameInputSwitchDownLeft: return -0.5f;
			case GameInputSwitchUpLeft: return 0.5f;
			}
		}

		return 0.0f;
	}

	void ReadMouseInput(IGameInputReading* reading, const GameInputMouseInfo* mouseInfo, InputDevice& device, uint32_t& currentChannel)
	{
		GameInputMouseState mouseState;
		if (!reading->GetMouseState(&mouseState))
			return;

		static constexpr uint32_t MaxMouseButtonCount = 7;
		for (uint32_t i = 0; i < MaxMouseButtonCount; i++)
		{
			uint32_t buttonID = 1 << i;

			// If the device doesn't have this mouse button we simply skip
			if (!(mouseInfo->supportedButtons & buttonID))
				continue;

			device.WriteChannelValue(currentChannel, (mouseState.buttons & buttonID) ? 1.0f : 0.0f);
			currentChannel++;
		}

		if (mouseInfo->hasWheelX)
		{
			device.WriteChannelValue(currentChannel, static_cast<float>(mouseState.wheelX));
			currentChannel++;
		}

		if (mouseInfo->hasWheelY)
		{
			device.WriteChannelValue(currentChannel, static_cast<float>(mouseState.wheelY));
			currentChannel++;
		}
	}

	void ReadKeyboardInput(IGameInputReading* reading, const GameInputKeyboardInfo* keyboardInfo, InputDevice& device, uint32_t& currentChannel)
	{
		static constexpr uint32_t MaxKeys = 512;
		static std::array<GameInputKeyState, MaxKeys> s_KeyStates;

		uint32_t readKeys = reading->GetKeyState(reading->GetKeyCount(), s_KeyStates.data());

		for (uint32_t keyChannel = currentChannel; keyChannel < currentChannel + keyboardInfo->keyCount; keyChannel++)
		{
			device.WriteChannelValue(keyChannel, 0.0f);
		}

		for (uint32_t keyIndex = 0; keyIndex < readKeys; keyIndex++)
		{
			device.WriteChannelValue(currentChannel + s_KeyStates[keyIndex].scanCode, 1.0f);
		}

		currentChannel += keyboardInfo->keyCount;
	}

	void ReadControllerAxisInput(IGameInputReading* reading, const GameInputControllerAxisInfo* axisInfo, InputDevice& device, uint32_t& currentChannel)
	{
		static constexpr uint32_t MaxAxes = 256;
		static std::array<float, MaxAxes> s_AxisValues;

		uint32_t readAxes = reading->GetControllerAxisState(MaxAxes, s_AxisValues.data());
		for (uint32_t i = 0; i < readAxes; i++)
		{
			device.WriteChannelValue(currentChannel, std::lerp(-1.0f, 1.0f, s_AxisValues[i]));
			currentChannel++;
		}
	}

	void ReadControllerButtonInput(IGameInputReading* reading, const GameInputControllerButtonInfo* buttonInfo, InputDevice& device, uint32_t& currentChannel)
	{
		static constexpr uint32_t MaxButtons = 512;
		static std::array<bool, MaxButtons> s_ButtonStates;

		uint32_t readButtons = reading->GetControllerButtonState(MaxButtons, s_ButtonStates.data());
		for (uint32_t i = 0; i < readButtons; i++)
		{
			device.WriteChannelValue(currentChannel, s_ButtonStates[i]);
			currentChannel++;
		}
	}

	void ReadControllerSwitchInput(IGameInputReading* reading, const GameInputControllerSwitchInfo* axisInfo, InputDevice& device, uint32_t& currentChannel)
	{
		static constexpr uint32_t MaxSwitches = 512;
		static std::array<GameInputSwitchPosition, MaxSwitches> s_SwitchPositions;

		uint32_t readSwitches = reading->GetControllerSwitchState(MaxSwitches, s_SwitchPositions.data());
		for (uint32_t i = 0; i < readSwitches; i++)
		{
			device.WriteChannelValue(currentChannel + 0, SwitchPositionToAxisValue(s_SwitchPositions[i], Axis::X));
			device.WriteChannelValue(currentChannel + 1, SwitchPositionToAxisValue(s_SwitchPositions[i], Axis::Y));
			currentChannel += 2;
		}
	}

	void InputDispatcher::Update()
	{
		for (auto& device : s_InputDevices)
		{
			IGameInputReading* reading;

			auto* nativeDevice = device.GetPrivateData<IGameInputDevice>();
			const auto* deviceInfo = nativeDevice->GetDeviceInfo();

			if (SUCCEEDED(s_GameInput->GetCurrentReading(deviceInfo->supportedInput, nativeDevice, &reading)))
			{
				uint32_t currentChannel = 0;

				auto inputKind = reading->GetInputKind();

				if (inputKind & GameInputKindMouse)
				{
					ReadMouseInput(reading, deviceInfo->mouseInfo, device, currentChannel);
				}

				if (inputKind & GameInputKindKeyboard)
				{
					ReadKeyboardInput(reading, deviceInfo->keyboardInfo, device, currentChannel);
				}

				if (inputKind & GameInputKindControllerAxis)
				{
					ReadControllerAxisInput(reading, deviceInfo->controllerAxisInfo, device, currentChannel);
				}

				if (inputKind & GameInputKindControllerButton)
				{
					ReadControllerButtonInput(reading, deviceInfo->controllerButtonInfo, device, currentChannel);
				}

				if (inputKind & GameInputKindControllerSwitch)
				{
					ReadControllerSwitchInput(reading, deviceInfo->controllerSwitchInfo, device, currentChannel);
				}

				reading->Release();
			}
		}
	}

	uint32_t InputDispatcher::GetDeviceCount() const
	{
		return static_cast<uint32_t>(s_InputDevices.size());
	}

	const InputDevice& InputDispatcher::GetDevice(uint32_t deviceIndex) const
	{
		if (deviceIndex >= s_InputDevices.size())
		{
			throw Exception("Index out of bounds trying to get input device!");
		}

		return s_InputDevices[deviceIndex];
	}

}
