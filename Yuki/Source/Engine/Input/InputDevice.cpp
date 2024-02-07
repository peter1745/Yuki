#include "InputDeviceImpl.hpp"

#include "Engine/Core/Exception.hpp"

namespace Yuki {

	void InputDevice::Impl::WriteChannelValue(uint32_t channelIndex, float32_t value)
	{
		if (channelIndex >= Channels.size())
		{
			auto msg = std::format("Tried writing to channel {} which is not a valid channel!", channelIndex);
			throw Exception(msg);
		}

		Channels[channelIndex].Value = value;
	}

	std::string_view InputDevice::GetName() const { return m_Impl->Name; }
	std::string_view InputDevice::GetManufacturerName() const { return m_Impl->ManufacturerName; }
	InputDevice::Type InputDevice::GetType() const { return m_Impl->Type; }

	const ExternalInputChannel* InputDevice::GetChannel(uint32_t channelIndex) const
	{
		if (channelIndex >= m_Impl->Channels.size())
		{
			return nullptr;
		}

		return &m_Impl->Channels[channelIndex];
	}

	InputDevice InputDeviceRegistry::GetDevice(InputDeviceID deviceID) const
	{
		if (!m_Impl->Devices.contains(deviceID))
		{
			return {};
		}

		return m_Impl->Devices.at(deviceID);
	}

	const std::unordered_map<InputDeviceID, InputDevice>& InputDeviceRegistry::GetAllDevices() const
	{
		return m_Impl->Devices;
	}

	InputDevice InputDeviceRegistry::Impl::CreateDevice(InputDeviceID deviceID)
	{
		if (Devices.contains(deviceID))
		{
			throw Exception("Cannot create InputDevice! Provided deviceID is already in use!");
		}

		InputDevice inputDevice = { new InputDevice::Impl() };
		Devices[deviceID] = inputDevice;
		return inputDevice;
	}

}
