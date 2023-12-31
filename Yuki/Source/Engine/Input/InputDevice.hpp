#pragma once

#include "Engine/Core/Exception.hpp"

#include "ExternalInputChannel.hpp"

namespace Yuki {

	inline constexpr uint32_t AnyMouseDevice =    ~0u - 1;
	inline constexpr uint32_t AnyKeyboardDevice = ~0u - 2;
	inline constexpr uint32_t AnyGamepadDevice =  ~0u - 3;

	class InputDevice
	{
	public:
		enum class Type
		{
			Unknown,
			Mouse,
			Keyboard,
			Gamepad,
			Controller
		};

		InputDevice(Type type, std::string_view name, std::string_view manufacturer, void* privateData)
			: m_Type(type), m_Name(name), m_Manufacturer(manufacturer), m_PrivateData(privateData)
		{
		}

		Type GetType() const { return m_Type; }
		std::string_view GetName() const { return m_Name; }
		std::string_view GetManufacturer() const { return m_Manufacturer; }

		void RegisterChannel()
		{
			m_Channels.emplace_back();
		}

		void WriteChannelValue(uint32_t channelIndex, float value)
		{
			if (channelIndex >= m_Channels.size())
			{
				auto msg = std::format("Tried writing to channel {} which is not a valid channel!", channelIndex);
				throw Exception(msg);
			}

			auto& channel = m_Channels[channelIndex];
			channel.PreviousValue = channel.Value;
			channel.Value = value;
		}

		const ExternalInputChannel* GetChannel(uint32_t channelIndex) const
		{
			if (channelIndex >= m_Channels.size())
			{
				return nullptr;
			}

			return &m_Channels[channelIndex];
		}

		float ReadChannelValue(uint32_t channelIndex) const
		{
			if (channelIndex >= m_Channels.size())
			{
				throw Exception(std::format("Tried reading from channel {} which is not a valid channel!", channelIndex));
			}

			return m_Channels[channelIndex].Value;
		}

		uint32_t GetChannelCount() const { return m_Channels.size(); }

	private:
		template<typename T>
		T* GetPrivateData() { return static_cast<T*>(m_PrivateData); }

	private:
		Type m_Type;
		std::string m_Name;
		std::string m_Manufacturer;
		std::vector<ExternalInputChannel> m_Channels;
		void* m_PrivateData;

		friend class InputAdapter;
	};

}
