#include "GameInputProvider.hpp"

#include "Engine/Input/InputDeviceImpl.hpp"
#include "Platform/Windows/WindowsCommon.hpp"

#include <GameInput.h>

#include <cfgmgr32.h>
#include <initguid.h>
#include <devpkey.h>
#include <hidsdi.h>

#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "Hid.lib")

namespace Yuki {

	static void CALLBACK DeviceCallback(
		GameInputCallbackToken callbackToken,
		void* context,
		IGameInputDevice* device,
		uint64_t timestamp,
		GameInputDeviceStatus currentStatus,
		GameInputDeviceStatus previousStatus);

	static float32_t SwitchPositionToAxisValue(GameInputSwitchPosition position, Axis axis);
	static std::pair<std::string, std::string> FetchDeviceNames(IGameInputDevice* device);
	
	void GameInputProvider::Init(InputDeviceRegistry registry)
	{
		m_DeviceRegistry = registry;

		CheckHR(GameInputCreate(&m_GameInput));

		CheckHR(m_GameInput->RegisterDeviceCallback(
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
			&m_DeviceCallbackToken
		));
	}

	void GameInputProvider::Update()
	{
		for (auto [deviceID, device] : m_Devices)
		{
			IGameInputReading* reading;

			const auto* deviceInfo = device->GetDeviceInfo();

			if (SUCCEEDED(m_GameInput->GetCurrentReading(deviceInfo->supportedInput, device, &reading)))
			{
				uint32_t currentChannel = 0;

				auto inputKind = reading->GetInputKind();

				if (inputKind & GameInputKindMouse)
				{
					ReadMouseInput(deviceID, reading, currentChannel);
				}

				if (inputKind & GameInputKindKeyboard)
				{
					ReadKeyboardInput(deviceID, reading, currentChannel);
				}

				if (inputKind & GameInputKindControllerAxis)
				{
					ReadControllerAxisInput(deviceID, reading, currentChannel);
				}

				if (inputKind & GameInputKindControllerButton)
				{
					ReadControllerButtonInput(deviceID, reading, currentChannel);
				}

				if (inputKind & GameInputKindControllerSwitch)
				{
					ReadControllerSwitchInput(deviceID, reading, currentChannel);
				}

				reading->Release();
			}
		}
	}

	void GameInputProvider::ReadMouseInput(InputDeviceID deviceID, IGameInputReading* reading, uint32_t& currentChannel)
	{
		GameInputMouseState mouseState;

		if (!reading->GetMouseState(&mouseState))
		{
			return;
		}

		auto device = m_DeviceRegistry.GetDevice(deviceID);
		const auto* deviceInfo = m_Devices.at(deviceID)->GetDeviceInfo();

		for (uint32_t i = 0; i < s_MaxMouseButtons; i++)
		{
			uint32_t buttonID = 1 << i;

			// If the device doesn't have this mouse button we simply skip
			if (!(deviceInfo->mouseInfo->supportedButtons & buttonID))
			{
				continue;
			}

			device->WriteChannelValue(currentChannel + i, (mouseState.buttons & buttonID) ? 1.0f : 0.0f);
		}
		currentChannel += s_MaxMouseButtons;

		if (deviceInfo->mouseInfo->hasWheelX)
		{
			int64_t delta = mouseState.wheelX - device->PreviousScrollX;
			device->WriteChannelValue(currentChannel, static_cast<float32_t>(delta));
			device->PreviousScrollX = mouseState.wheelX;
		}
		currentChannel++;

		if (deviceInfo->mouseInfo->hasWheelY)
		{
			int64_t delta = mouseState.wheelY - device->PreviousScrollY;
			device->WriteChannelValue(currentChannel, static_cast<float32_t>(delta));
			device->PreviousScrollY = mouseState.wheelY;
		}
		currentChannel++;
	}

	void GameInputProvider::ReadKeyboardInput(InputDeviceID deviceID, IGameInputReading* reading, uint32_t& currentChannel)
	{
		static std::array<GameInputKeyState, s_MaxKeyCount> s_KeyStates;

		auto device = m_DeviceRegistry.GetDevice(deviceID);
	
		for (uint32_t i = 0; i < s_MaxKeyCount; i++)
		{
			device->WriteChannelValue(currentChannel + i, 0.0f);
		}

		uint32_t readKeys = reading->GetKeyState(s_MaxKeyCount, s_KeyStates.data());
		for (uint32_t i = 0; i < readKeys; i++)
		{
			device->WriteChannelValue(currentChannel + s_KeyStates[i].virtualKey, 1.0f);
		}

		currentChannel += s_MaxKeyCount;
	}

	void GameInputProvider::ReadControllerAxisInput(InputDeviceID deviceID, IGameInputReading* reading, uint32_t& currentChannel)
	{
		static std::array<float32_t, s_MaxControllerAxes> s_AxisValues;

		auto device = m_DeviceRegistry.GetDevice(deviceID);

		uint32_t axisCount = reading->GetControllerAxisCount();
		YukiUnused(reading->GetControllerAxisState(s_MaxControllerAxes, s_AxisValues.data()));
		for (uint32_t i = 0; i < axisCount; i++)
		{
			device->WriteChannelValue(currentChannel + i, std::lerp(-1.0f, 1.0f, s_AxisValues[i]));
		}

		currentChannel += axisCount;
	}

	void GameInputProvider::ReadControllerButtonInput(InputDeviceID deviceID, IGameInputReading* reading, uint32_t& currentChannel)
	{
		static std::array<bool, s_MaxControllerButtons> s_ButtonStates;

		auto device = m_DeviceRegistry.GetDevice(deviceID);

		uint32_t buttonCount = reading->GetControllerButtonCount();
		YukiUnused(reading->GetControllerButtonState(s_MaxControllerButtons, s_ButtonStates.data()));
		for (uint32_t buttonIndex = 0; buttonIndex < buttonCount; buttonIndex++)
		{
			device->WriteChannelValue(currentChannel + buttonIndex, s_ButtonStates[buttonIndex] ? 1.0f : 0.0f);
		}

		currentChannel += buttonCount;
	}

	void GameInputProvider::ReadControllerSwitchInput(InputDeviceID deviceID, IGameInputReading* reading, uint32_t& currentChannel)
	{
		static std::array<GameInputSwitchPosition, s_MaxControllerSwitches> s_SwitchPositions;

		auto device = m_DeviceRegistry.GetDevice(deviceID);

		uint32_t switchCount = reading->GetControllerSwitchCount();
		YukiUnused(reading->GetControllerSwitchState(s_MaxControllerSwitches, s_SwitchPositions.data()));
		for (uint32_t switchIndex = 0; switchIndex < switchCount * 2; switchIndex += 2)
		{
			device->WriteChannelValue(currentChannel + switchIndex, SwitchPositionToAxisValue(s_SwitchPositions[switchIndex], Axis::X));
			device->WriteChannelValue(currentChannel + switchIndex + 1, SwitchPositionToAxisValue(s_SwitchPositions[switchIndex + 1], Axis::Y));
		}

		currentChannel += switchCount;
	}

	uint32_t g_DummyID = 0;
	void GameInputProvider::RegisterGameInputDevice(IGameInputDevice* device)
	{
		auto [deviceName, manufacturer] = FetchDeviceNames(device);

		InputDeviceID deviceID = g_DummyID++; // Generate from GameInput deviceId (hash or something)

		InputDevice inputDevice = m_DeviceRegistry->CreateDevice(deviceID);
		inputDevice->Name = deviceName;
		inputDevice->ManufacturerName = manufacturer;

		const auto* deviceInfo = device->GetDeviceInfo();

		uint32_t requiredChannels = 0;

		if (deviceInfo->supportedInput & GameInputKindMouse)
		{
			requiredChannels += s_MaxMouseChannels;
			inputDevice->Type = InputDevice::Type::Mouse;
			inputDevice->HasScrollX = deviceInfo->mouseInfo->hasWheelX;
			inputDevice->HasScrollY = deviceInfo->mouseInfo->hasWheelY;
		}

		if (deviceInfo->supportedInput & GameInputKindKeyboard)
		{
			requiredChannels += s_MaxKeyCount;
			inputDevice->Type = InputDevice::Type::Keyboard;
		}

		if (deviceInfo->supportedInput & GameInputKindGamepad)
			inputDevice->Type = InputDevice::Type::Gamepad;
		else if (deviceInfo->supportedInput & GameInputKindController)
			inputDevice->Type = InputDevice::Type::Controller;

		if (deviceInfo->supportedInput & GameInputKindControllerAxis)
			requiredChannels += deviceInfo->controllerAxisCount;

		if (deviceInfo->supportedInput & GameInputKindControllerButton)
			requiredChannels += deviceInfo->controllerButtonCount;

		if (deviceInfo->supportedInput & GameInputKindControllerSwitch)
			requiredChannels += deviceInfo->controllerSwitchCount * 2;

		inputDevice->Channels.resize(requiredChannels, { 0.0f, 0.0f });

		m_Devices[deviceID] = device;
	}

	void GameInputProvider::UnregisterGameInputDevice(IGameInputDevice* device)
	{
		std::erase_if(m_Devices, [device](const auto& keyValue)
		{
			return keyValue.second == device;
		});

		device->Release();
	}

	float32_t SwitchPositionToAxisValue(GameInputSwitchPosition position, Axis axis)
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

	void CALLBACK DeviceCallback(
		GameInputCallbackToken callbackToken,
		void* context,
		IGameInputDevice* device,
		uint64_t timestamp,
		GameInputDeviceStatus currentStatus,
		GameInputDeviceStatus previousStatus)
	{
		auto* provider = static_cast<GameInputProvider*>(context);

		if (currentStatus & GameInputDeviceConnected)
		{
			provider->RegisterGameInputDevice(device);
		}
		else
		{
			provider->UnregisterGameInputDevice(device);
		}
	}

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

}
