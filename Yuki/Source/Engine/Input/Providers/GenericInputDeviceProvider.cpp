#include "GenericInputDeviceProvider.hpp"
#include "Engine/Input/InputDeviceImpl.hpp"

namespace Yuki {

	void GenericInputDeviceProvider::Init(InputDeviceRegistry registry)
	{
		m_DeviceRegistry = registry;

		auto createGenericInputDevice = [registry](InputDeviceID deviceID, std::string_view type, uint32_t channelCount)
		{
			auto inputDevice = registry->CreateDevice(deviceID);
			inputDevice->Name = std::format("Internal {} Device", type);
			inputDevice->ManufacturerName = "None";

			for (uint32_t i = 0; i < channelCount; i++)
				inputDevice->Channels.push_back({});
		};

		createGenericInputDevice(GenericMouse, "Mouse", s_MaxMouseChannels);
		createGenericInputDevice(GenericKeyboard, "Keyboard", s_MaxKeyCount);
		createGenericInputDevice(GenericGamepad, "Gamepad", s_MaxGamepadChannels);
	}

	void GenericInputDeviceProvider::Update()
	{
		{
			// Update generic mouse channels
			auto genericMouse = m_DeviceRegistry->Devices.at(GenericMouse);
			std::array<float32_t, s_MaxMouseChannels> genericMouseValues;
			genericMouseValues.fill(0.0f);
			for (auto [deviceID, device] : m_DeviceRegistry->Devices)
			{
				if (device->Type != InputDevice::Type::Mouse || deviceID == GenericMouse)
				{
					continue;
				}

				for (uint32_t i = 0; i < s_MaxMouseButtons; i++)
				{
					if (device->Channels[i].Value > 0.0f)
						genericMouseValues[i] = 1.0f;
				}

				if (device->HasScrollX)
				{
					float32_t value = device->Channels[s_MaxMouseChannels - 2].Value;

					if (value != 0.0f)
						genericMouseValues[s_MaxMouseChannels - 2] = value;
				}

				if (device->HasScrollY)
				{
					float32_t value = device->Channels[s_MaxMouseChannels - 1].Value;

					if (value != 0.0f)
						genericMouseValues[s_MaxMouseChannels - 1] = value;
				}
			}

			for (uint32_t i = 0; i < s_MaxMouseChannels; i++)
				genericMouse->WriteChannelValue(i, genericMouseValues[i]);
		}

		{
			// Update generic keyboard buttons
			auto genericKeyboard = m_DeviceRegistry->Devices.at(GenericKeyboard);
			std::array<float32_t, s_MaxKeyCount> genericKeyboardButtons;
			genericKeyboardButtons.fill(0.0f);
			for (auto [deviceID, device] : m_DeviceRegistry->Devices)
			{
				if (device->Type != InputDevice::Type::Keyboard || deviceID == GenericKeyboard)
				{
					continue;
				}

				for (uint32_t i = 0; i < s_MaxKeyCount; i++)
				{
					if (device->Channels[i].Value > 0.0f)
						genericKeyboardButtons[i] = 1.0f;
				}
			}

			for (uint32_t i = 0; i < s_MaxKeyCount; i++)
				genericKeyboard->WriteChannelValue(i, genericKeyboardButtons[i]);
		}

		{
			// Update generic gamepad channels
			auto genericGamepad = m_DeviceRegistry->Devices.at(GenericGamepad);
			std::array<float32_t, s_MaxGamepadChannels> genericGamepadValues;
			genericGamepadValues.fill(0.0f);
			for (auto [deviceID, device] : m_DeviceRegistry->Devices)
			{
				if (device->Type != InputDevice::Type::Gamepad || deviceID == GenericGamepad)
				{
					continue;
				}

				for (uint32_t i = 0; i < s_MaxGamepadChannels; i++)
				{
					if (i >= device->Channels.size())
						break;

					if (device->Channels[i].Value != 0.0f)
						genericGamepadValues[i] = device->Channels[i].Value;
				}
			}

			for (uint32_t i = 0; i < s_MaxGamepadChannels; i++)
				genericGamepad->WriteChannelValue(i, genericGamepadValues[i]);
		}
	}

}
