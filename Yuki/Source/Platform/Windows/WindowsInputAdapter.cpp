#include "Engine/Input/InputAdapter.hpp"

#include "Engine/Core/Core.hpp"
#include "Engine/Core/Unique.hpp"

#include "WindowsCommon.hpp"

#include <GameInput.h>

#include <initguid.h>
#include <cfgmgr32.h>
#include <devpkey.h>
#include <hidsdi.h>

#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "Hid.lib")

template<>
struct std::hash<APP_LOCAL_DEVICE_ID>
{
	size_t operator()(const APP_LOCAL_DEVICE_ID& id) const
	{
		size_t hash = 0;
		uint32_t componentSize = APP_LOCAL_DEVICE_ID_SIZE / 4;

		for (uint32_t i = 0; i < 4; i++)
		{
			size_t component = 0;
			memcpy(&component, &id.value[i * componentSize], componentSize);
			hash += component;
		}

		return hash;
	}
};

static bool operator==(const APP_LOCAL_DEVICE_ID& id0, const APP_LOCAL_DEVICE_ID& id1)
{
	return memcmp(id0.value, id1.value, APP_LOCAL_DEVICE_ID_SIZE) == 0;
}

namespace Yuki {

	static IGameInput* s_GameInput;
	static GameInputCallbackToken s_DeviceCallbackToken;
	static std::unordered_map<APP_LOCAL_DEVICE_ID, Unique<InputDevice>> s_InputDevices;

	static Unique<InputDevice> s_GenericKeyboardDevice;
	static Unique<InputDevice> s_GenericGamepadDevice;

	// Ugly mess to deal with the fact that GameInput doesn't provide us with a user friendly name currently
	static std::pair<std::string, std::string> FetchDeviceNames(IGameInputDevice* device)
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

		uint32_t rawDeviceCount = 0;
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

			wchar_t friendlyName[1024]{};
			wchar_t manufacturerName[1024]{};

			HANDLE h = CreateFile(devicePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			DEVPROPTYPE propertyType;
			ULONG propertySize = 0;
			CM_Get_Device_Interface_PropertyW(devicePath.c_str(), &DEVPKEY_Device_InstanceId, &propertyType, nullptr, &propertySize, 0);

			std::wstring deviceId;
			deviceId.resize(propertySize);
			CM_Get_Device_Interface_PropertyW(devicePath.c_str(), &DEVPKEY_Device_InstanceId, &propertyType, reinterpret_cast<PBYTE>(deviceId.data()), &propertySize, 0);

			DEVINST devInst;
			CM_Locate_DevNodeW(&devInst, reinterpret_cast<DEVINSTID>(deviceId.data()), CM_LOCATE_DEVNODE_NORMAL);

			if (!HidD_GetProductString(h, friendlyName, 1024))
			{
				CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_NAME, &propertyType, nullptr, &propertySize, 0);
				CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_NAME, &propertyType, reinterpret_cast<PBYTE>(friendlyName), &propertySize, 0);
			}

			if (!HidD_GetManufacturerString(h, manufacturerName, 1024))
			{
				CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_Manufacturer, &propertyType, nullptr, &propertySize, 0);
				CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_Manufacturer, &propertyType, reinterpret_cast<PBYTE>(manufacturerName), &propertySize, 0);
			}

			if (deviceInfo->vendorId != ridDeviceInfo.hid.dwVendorId || deviceInfo->productId != ridDeviceInfo.hid.dwProductId)
			{
				continue;
			}

			CloseHandle(h);

			return { Utf16ToUtf8(friendlyName), Utf16ToUtf8(manufacturerName) };
		}

		return { "Generic HID Device", "Unknown Manufacturer" };
	}

	static constexpr uint32_t s_MaxKeyCount = 256;

	static void CALLBACK DeviceCallback(
		GameInputCallbackToken callbackToken,
		void* context,
		IGameInputDevice* device,
		uint64_t timestamp,
		GameInputDeviceStatus currentStatus,
		GameInputDeviceStatus previousStatus)
	{
		const auto* deviceInfo = device->GetDeviceInfo();

		if (!(currentStatus & GameInputDeviceConnected))
		{
			if (!s_InputDevices.contains(deviceInfo->deviceId))
				return;

			s_InputDevices.erase(deviceInfo->deviceId);
		}
		else
		{
			auto[deviceName, manufacturer] = FetchDeviceNames(device);

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

			s_InputDevices[deviceInfo->deviceId] = Unique<InputDevice>::New(deviceType, deviceName, manufacturer, device);

			auto& inputDevice = s_InputDevices[deviceInfo->deviceId];

			if (deviceInfo->mouseInfo)
			{
				uint32_t buttonCount = std::popcount(static_cast<uint32_t>(deviceInfo->mouseInfo->supportedButtons));
				buttonCount += deviceInfo->mouseInfo->hasWheelX;
				buttonCount += deviceInfo->mouseInfo->hasWheelY;

				for (uint32_t i = 0; i < buttonCount; i++)
				{
					inputDevice->RegisterChannel();
				}
			}

			if (deviceInfo->keyboardInfo)
			{
				for (uint32_t i = 0; i < s_MaxKeyCount; i++)
				{
					inputDevice->RegisterChannel();
				}
			}

			for (uint32_t i = 0; i < deviceInfo->controllerAxisCount; i++)
			{
				inputDevice->RegisterChannel();
			}

			for (uint32_t i = 0; i < deviceInfo->controllerButtonCount; i++)
			{
				inputDevice->RegisterChannel();
			}

			for (uint32_t i = 0; i < deviceInfo->controllerSwitchCount; i++)
			{
				inputDevice->RegisterChannel();
				inputDevice->RegisterChannel();
			}

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

			std::cout << "Channel Count: " << inputDevice->GetChannelCount() << "\n";
		}
	}

	InputAdapter::InputAdapter()
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
			nullptr,
			DeviceCallback,
			&s_DeviceCallbackToken
		));

		auto createGenericInputDevice = [](std::string_view type, uint32_t channelCount)
		{
			auto inputDevice = Unique<InputDevice>::New(
				InputDevice::Type::Keyboard,
				"Internal Keyboard Device",
				"None",
				nullptr
			);

			for (uint32_t i = 0; i < channelCount; i++)
				inputDevice->RegisterChannel();

			return inputDevice;
		};

		s_GenericKeyboardDevice = createGenericInputDevice("Keyboard", s_MaxKeyCount);
		s_GenericGamepadDevice = createGenericInputDevice("Gamepad", 128);
	}

	InputAdapter::~InputAdapter()
	{
		s_GameInput->UnregisterCallback(s_DeviceCallbackToken, 0);
		s_GameInput->Release();
	}

	static float SwitchPositionToAxisValue(GameInputSwitchPosition position, Axis axis)
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

	static void ReadMouseInput(IGameInputReading* reading, const GameInputMouseInfo* mouseInfo, InputDevice& device, uint32_t& currentChannel)
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

	static void ReadKeyboardInput(IGameInputReading* reading, const GameInputKeyboardInfo* keyboardInfo, InputDevice& device, uint32_t& currentChannel)
	{
		static std::array<GameInputKeyState, s_MaxKeyCount> s_KeyStates;
		
		for (uint32_t i = 0; i < s_MaxKeyCount; i++)
		{
			device.WriteChannelValue(currentChannel + i, 0.0f);
		}

		uint32_t readKeys = reading->GetKeyState(s_MaxKeyCount, s_KeyStates.data());
		for (uint32_t i = 0; i < readKeys; i++)
		{
			device.WriteChannelValue(currentChannel + s_KeyStates[i].virtualKey, 1.0f);
			s_GenericKeyboardDevice->WriteChannelValue(s_KeyStates[i].virtualKey, 1.0f);
		}

		// NOTE(Peter): Specifically not using `reading` here because the keyboard implementation for GameInput
		//				doesn't let us easily index a channel based on the key
		/*for (uint32_t i = 0; i < s_MaxKeyCount; i++)
		{
			bool state = GetAsyncKeyState(i) & 0x8000;
			device.WriteChannelValue(currentChannel + i, AxisValue1D{ state ? 1.0f : 0.0f });
		}*/

		currentChannel += s_MaxKeyCount;
	}

	static void ReadControllerAxisInput(IGameInputReading* reading, const GameInputControllerAxisInfo* axisInfo, InputDevice& device, uint32_t& currentChannel)
	{
		static constexpr uint32_t MaxAxes = 256;
		static std::array<float, MaxAxes> s_AxisValues;

		bool isGamepad = reading->GetInputKind() & GameInputKindGamepad;

		uint32_t axisCount = reading->GetControllerAxisCount();
		YukiUnused(reading->GetControllerAxisState(MaxAxes, s_AxisValues.data()));
		for (uint32_t i = 0; i < axisCount; i++)
		{
			if (isGamepad)
			{
				s_GenericGamepadDevice->WriteChannelValue(currentChannel + i, std::lerp(-1.0f, 1.0f, s_AxisValues[i]));
			}

			device.WriteChannelValue(currentChannel + i, std::lerp(-1.0f, 1.0f, s_AxisValues[i]));
		}

		currentChannel += axisCount;
	}

	static void ReadControllerButtonInput(IGameInputReading* reading, const GameInputControllerButtonInfo* buttonInfo, InputDevice& device, uint32_t& currentChannel)
	{
		static constexpr uint32_t MaxButtons = 512;
		static std::array<bool, MaxButtons> s_ButtonStates;

		bool isGamepad = reading->GetInputKind() & GameInputKindGamepad;

		uint32_t buttonCount = reading->GetControllerButtonCount();
		YukiUnused(reading->GetControllerButtonState(MaxButtons, s_ButtonStates.data()));
		for (uint32_t buttonIndex = 0; buttonIndex < buttonCount; buttonIndex++)
		{
			if (isGamepad)
			{
				s_GenericGamepadDevice->WriteChannelValue(currentChannel + buttonIndex, s_ButtonStates[buttonIndex] ? 1.0f : 0.0f);
			}

			device.WriteChannelValue(currentChannel + buttonIndex, s_ButtonStates[buttonIndex] ? 1.0f : 0.0f);
		}

		currentChannel += buttonCount;
	}

	static void ReadControllerSwitchInput(IGameInputReading* reading, const GameInputControllerSwitchInfo* axisInfo, InputDevice& device, uint32_t& currentChannel)
	{
		static constexpr uint32_t MaxSwitches = 512;
		static std::array<GameInputSwitchPosition, MaxSwitches> s_SwitchPositions;

		bool isGamepad = reading->GetInputKind() & GameInputKindGamepad;

		uint32_t switchCount = reading->GetControllerSwitchCount();
		YukiUnused(reading->GetControllerSwitchState(switchCount, s_SwitchPositions.data()));
		for (uint32_t switchIndex = 0; switchIndex < switchCount * 2; switchIndex += 2)
		{
			if (isGamepad)
			{
				s_GenericGamepadDevice->WriteChannelValue(currentChannel + switchIndex, SwitchPositionToAxisValue(s_SwitchPositions[switchIndex], Axis::X));
				s_GenericGamepadDevice->WriteChannelValue(currentChannel + switchIndex + 1, SwitchPositionToAxisValue(s_SwitchPositions[switchIndex + 1], Axis::Y));
			}

			device.WriteChannelValue(currentChannel + switchIndex, SwitchPositionToAxisValue(s_SwitchPositions[switchIndex], Axis::X));
			device.WriteChannelValue(currentChannel + switchIndex + 1, SwitchPositionToAxisValue(s_SwitchPositions[switchIndex + 1], Axis::Y));
		}

		currentChannel += switchCount;
	}

	void InputAdapter::Update()
	{
		for (uint32_t i = 0; i < s_MaxKeyCount; i++)
		{
			s_GenericKeyboardDevice->WriteChannelValue(i, 0.0f);
		}

		for (auto& [deviceID, device] : s_InputDevices)
		{
			IGameInputReading* reading;

			auto* nativeDevice = device->GetPrivateData<IGameInputDevice>();
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

	uint32_t InputAdapter::GetDeviceCount() const
	{
		return static_cast<uint32_t>(s_InputDevices.size());
	}

	const InputDevice& InputAdapter::GetDevice(uint32_t deviceIndex) const
	{
		if (deviceIndex == AnyKeyboardDevice)
		{
			return s_GenericKeyboardDevice;
		}
		else if (deviceIndex == AnyGamepadDevice)
		{
			return s_GenericGamepadDevice;
		}

		if (deviceIndex >= s_InputDevices.size())
		{
			throw Exception("Index out of bounds trying to get input device!");
		}

		uint32_t i = 0;
		for (const auto& [deviceID, device] : s_InputDevices)
		{
			if (i == deviceIndex)
				return device;

			i++;
		}
	}

}
