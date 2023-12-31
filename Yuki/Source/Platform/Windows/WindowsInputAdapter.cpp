#include "WindowsInputAdapter.hpp"

#include "WindowsCommon.hpp"

#include <initguid.h>
#include <cfgmgr32.h>
#include <devpkey.h>
#include <hidsdi.h>

#pragma comment(lib, "cfgmgr32.lib")
#pragma comment(lib, "Hid.lib")

namespace Yuki {

	static constexpr uint32_t s_MaxKeyCount = 256;

	// Max supported mouse buttons according to GameInput + scroll wheel x and y
	static constexpr uint32_t s_MaxMouseChannels = 7 + 2;

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

	static void CALLBACK DeviceCallback(
		GameInputCallbackToken callbackToken,
		void* context,
		IGameInputDevice* device,
		uint64_t timestamp,
		GameInputDeviceStatus currentStatus,
		GameInputDeviceStatus previousStatus)
	{
		InputAdapter::Impl* impl = static_cast<InputAdapter::Impl*>(context);

		if (!(currentStatus & GameInputDeviceConnected))
		{
			impl->UnregisterDevice(device);
		}
		else
		{
			impl->RegisterDevice(device);
		}
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

	std::string_view InputDevice::GetName() const { return m_Impl->Name; }
	std::string_view InputDevice::GetManufacturerName() const { return m_Impl->ManufacturerName; }

	const ExternalInputChannel* InputDevice::GetChannel(uint32_t channelIndex) const
	{
		if (channelIndex >= m_Impl->Channels.size())
		{
			return nullptr;
		}

		return &m_Impl->Channels[channelIndex];
	}

	void InputDevice::Impl::ReadMouseInput(IGameInputReading* reading, uint32_t& currentChannel)
	{
		GameInputMouseState mouseState;

		if (!reading->GetMouseState(&mouseState))
			return;

		auto genericMouse = Adapter->GenericDevices[AnyMouseDevice];

		for (uint32_t i = 0; i < s_MaxMouseChannels - 2; i++)
		{
			uint32_t buttonID = 1 << i;

			// If the device doesn't have this mouse button we simply skip
			if (!(Info->mouseInfo->supportedButtons & buttonID))
				continue;

			genericMouse->WriteChannelValue(currentChannel + i, (mouseState.buttons & buttonID) ? 1.0f : 0.0f);
			WriteChannelValue(currentChannel + i, (mouseState.buttons & buttonID) ? 1.0f : 0.0f);
		}
		currentChannel += s_MaxMouseChannels - 2;

		if (Info->mouseInfo->hasWheelX)
		{
			int64_t delta = mouseState.wheelX - PreviousScrollX;
			genericMouse->WriteChannelValue(currentChannel, static_cast<float>(delta));
			WriteChannelValue(currentChannel, static_cast<float>(delta));
			PreviousScrollX = mouseState.wheelX;
		}
		currentChannel++;

		if (Info->mouseInfo->hasWheelY)
		{
			int64_t delta = mouseState.wheelY - PreviousScrollY;
			genericMouse->WriteChannelValue(currentChannel, static_cast<float>(delta));
			WriteChannelValue(currentChannel, static_cast<float>(delta));
			PreviousScrollY = mouseState.wheelY;
		}
		currentChannel++;
	}

	void InputDevice::Impl::ReadKeyboardInput(IGameInputReading* reading, uint32_t& currentChannel)
	{
		static std::array<GameInputKeyState, s_MaxKeyCount> s_KeyStates;

		for (uint32_t i = 0; i < s_MaxKeyCount; i++)
		{
			WriteChannelValue(currentChannel + i, 0.0f);
		}

		auto genericKeyboard = Adapter->GenericDevices[AnyKeyboardDevice];

		uint32_t readKeys = reading->GetKeyState(s_MaxKeyCount, s_KeyStates.data());
		for (uint32_t i = 0; i < readKeys; i++)
		{
			genericKeyboard->WriteChannelValue(s_KeyStates[i].virtualKey, 1.0f);
			WriteChannelValue(currentChannel + s_KeyStates[i].virtualKey, 1.0f);
		}

		currentChannel += s_MaxKeyCount;
	}

	void InputDevice::Impl::ReadControllerAxisInput(IGameInputReading* reading, uint32_t& currentChannel)
	{
		static constexpr uint32_t MaxAxes = 256;
		static std::array<float, MaxAxes> s_AxisValues;

		auto genericGamepad = Adapter->GenericDevices[AnyGamepadDevice];
		bool isGamepad = reading->GetInputKind() & GameInputKindGamepad;

		uint32_t axisCount = reading->GetControllerAxisCount();
		YukiUnused(reading->GetControllerAxisState(MaxAxes, s_AxisValues.data()));
		for (uint32_t i = 0; i < axisCount; i++)
		{
			if (isGamepad)
			{
				genericGamepad->WriteChannelValue(currentChannel + i, std::lerp(-1.0f, 1.0f, s_AxisValues[i]));
			}

			WriteChannelValue(currentChannel + i, std::lerp(-1.0f, 1.0f, s_AxisValues[i]));
		}

		currentChannel += axisCount;
	}

	void InputDevice::Impl::ReadControllerButtonInput(IGameInputReading* reading, uint32_t& currentChannel)
	{
		static constexpr uint32_t MaxButtons = 512;
		static std::array<bool, MaxButtons> s_ButtonStates;

		auto genericGamepad = Adapter->GenericDevices[AnyGamepadDevice];
		bool isGamepad = reading->GetInputKind() & GameInputKindGamepad;

		uint32_t buttonCount = reading->GetControllerButtonCount();
		YukiUnused(reading->GetControllerButtonState(MaxButtons, s_ButtonStates.data()));
		for (uint32_t buttonIndex = 0; buttonIndex < buttonCount; buttonIndex++)
		{
			if (isGamepad)
			{
				genericGamepad->WriteChannelValue(currentChannel + buttonIndex, s_ButtonStates[buttonIndex] ? 1.0f : 0.0f);
			}

			WriteChannelValue(currentChannel + buttonIndex, s_ButtonStates[buttonIndex] ? 1.0f : 0.0f);
		}

		currentChannel += buttonCount;
	}

	void InputDevice::Impl::ReadControllerSwitchInput(IGameInputReading* reading, uint32_t& currentChannel)
	{
		static constexpr uint32_t MaxSwitches = 512;
		static std::array<GameInputSwitchPosition, MaxSwitches> s_SwitchPositions;

		auto genericGamepad = Adapter->GenericDevices[AnyGamepadDevice];
		bool isGamepad = reading->GetInputKind() & GameInputKindGamepad;

		uint32_t switchCount = reading->GetControllerSwitchCount();
		YukiUnused(reading->GetControllerSwitchState(switchCount, s_SwitchPositions.data()));
		for (uint32_t switchIndex = 0; switchIndex < switchCount * 2; switchIndex += 2)
		{
			if (isGamepad)
			{
				genericGamepad->WriteChannelValue(currentChannel + switchIndex, SwitchPositionToAxisValue(s_SwitchPositions[switchIndex], Axis::X));
				genericGamepad->WriteChannelValue(currentChannel + switchIndex + 1, SwitchPositionToAxisValue(s_SwitchPositions[switchIndex + 1], Axis::Y));
			}

			WriteChannelValue(currentChannel + switchIndex, SwitchPositionToAxisValue(s_SwitchPositions[switchIndex], Axis::X));
			WriteChannelValue(currentChannel + switchIndex + 1, SwitchPositionToAxisValue(s_SwitchPositions[switchIndex + 1], Axis::Y));
		}

		currentChannel += switchCount;
	}

	void InputDevice::Impl::WriteChannelValue(uint32_t channelIndex, float value)
	{
		if (channelIndex >= Channels.size())
		{
			auto msg = std::format("Tried writing to channel {} which is not a valid channel!", channelIndex);
			throw Exception(msg);
		}

		auto& channel = Channels[channelIndex];
		channel.PreviousValue = channel.Value;
		channel.Value = value;
	}

	InputAdapter InputAdapter::Create()
	{
		auto* impl = new Impl();

		CheckHR(GameInputCreate(&impl->Context));

		CheckHR(impl->Context->RegisterDeviceCallback(
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
			impl,
			DeviceCallback,
			&impl->DeviceCallbackToken
		));

		auto createGenericInputDevice = [&](uint32_t deviceID, std::string_view type, uint32_t channelCount)
		{
			auto inputDevice = InputDevice{ new InputDevice::Impl() };
			inputDevice->Name = std::format("Internal {} Device", type);
			inputDevice->ManufacturerName = "None";

			for (uint32_t i = 0; i < channelCount; i++)
				inputDevice->Channels.push_back({});

			impl->GenericDevices[deviceID] = inputDevice;
		};

		createGenericInputDevice(AnyMouseDevice, "Mouse", s_MaxMouseChannels);
		createGenericInputDevice(AnyKeyboardDevice, "Keyboard", s_MaxKeyCount);
		createGenericInputDevice(AnyGamepadDevice, "Gamepad", 128);

		return { impl };
	}

	void InputAdapter::Destroy()
	{
		m_Impl->Context->UnregisterCallback(m_Impl->DeviceCallbackToken, 0);
		m_Impl->Context->Release();

		delete m_Impl;
	}

	void InputAdapter::Impl::RegisterDevice(IGameInputDevice* device)
	{
		auto [deviceName, manufacturer] = FetchDeviceNames(device);

		InputDeviceID deviceID = static_cast<InputDeviceID>(Devices.size());

		auto* deviceImpl = new InputDevice::Impl();
		deviceImpl->Adapter = { this };
		deviceImpl->Device = device;
		deviceImpl->Info = device->GetDeviceInfo();
		deviceImpl->Name = deviceName;
		deviceImpl->ManufacturerName = manufacturer;

		const auto* deviceInfo = device->GetDeviceInfo();

		uint32_t requiredChannels = 0;

		if (deviceInfo->supportedInput & GameInputKindMouse)
			requiredChannels += s_MaxMouseChannels;
		
		if (deviceInfo->supportedInput & GameInputKindKeyboard)
			requiredChannels += s_MaxKeyCount;

		if (deviceInfo->supportedInput & GameInputKindControllerAxis)
			requiredChannels += deviceInfo->controllerAxisCount;

		if (deviceInfo->supportedInput & GameInputKindControllerButton)
			requiredChannels += deviceInfo->controllerButtonCount;

		if (deviceInfo->supportedInput & GameInputKindControllerSwitch)
			requiredChannels += deviceInfo->controllerSwitchCount * 2;

		deviceImpl->Channels.resize(requiredChannels, { 0.0f, 0.0f });

		Devices[deviceID] = { deviceImpl };
	}

	void InputAdapter::Impl::UnregisterDevice(IGameInputDevice* device)
	{
		std::erase_if(Devices, [device](const auto& keyValue)
		{
			return keyValue.second->Device == device;
		});

		// TODO(Peter): Verify that this is necessary
		device->Release();
	}

	void InputAdapter::Update() const
	{
		auto genericKeyboard = m_Impl->GenericDevices[AnyKeyboardDevice];
		for (uint32_t i = 0; i < s_MaxKeyCount; i++)
		{
			genericKeyboard->WriteChannelValue(i, 0.0f);
		}

		for (auto device : m_Impl->Devices | std::views::values)
		{
			IGameInputReading* reading;

			if (SUCCEEDED(m_Impl->Context->GetCurrentReading(device->Info->supportedInput, device->Device, &reading)))
			{
				uint32_t currentChannel = 0;

				auto inputKind = reading->GetInputKind();

				if (inputKind & GameInputKindMouse)
				{
					device->ReadMouseInput(reading, currentChannel);
				}

				if (inputKind & GameInputKindKeyboard)
				{
					device->ReadKeyboardInput(reading, currentChannel);
				}

				if (inputKind & GameInputKindControllerAxis)
				{
					device->ReadControllerAxisInput(reading, currentChannel);
				}

				if (inputKind & GameInputKindControllerButton)
				{
					device->ReadControllerButtonInput(reading, currentChannel);
				}

				if (inputKind & GameInputKindControllerSwitch)
				{
					device->ReadControllerSwitchInput(reading, currentChannel);
				}

				reading->Release();
			}
		}
	}

	const InputDevice InputAdapter::GetDevice(uint32_t deviceIndex) const
	{
		if (m_Impl->GenericDevices.contains(deviceIndex))
		{
			return m_Impl->GenericDevices.at(deviceIndex);
		}

		if (m_Impl->Devices.contains(deviceIndex))
		{
			return m_Impl->Devices.at(deviceIndex);
		}

		return {};
	}

}
