#pragma once

#include "Engine/Core/Exception.hpp"

#include <format>
#include <string>
#include <algorithm>

namespace Yuki {

	enum class Axis
	{
		None,
		X,
		Y
	};

	struct ChannelValue
	{
		float Value;

		operator float() const { return Value; }

		ChannelValue& operator=(const float value)
		{
			Value = std::clamp(value, -1.0f, 1.0f);
			return *this;
		}
	};

	struct ExternalInputChannel
	{
		ChannelValue Value;
	};

	class InputDevice
	{
	public:
		enum class Type
		{
			Unknown,
			Mouse,
			Keyboard,
			Controller
		};

		InputDevice(Type type, std::string_view name, std::string_view manufacturer, uint32_t channelCount, void* privateData)
			: m_Type(type), m_Name(name), m_Manufacturer(manufacturer), m_PrivateData(privateData)
		{
			m_Channels.resize(channelCount);
		}

		Type GetType() const { return m_Type; }
		std::string_view GetName() const { return m_Name; }
		std::string_view GetManufacturer() const { return m_Manufacturer; }

		void WriteChannelValue(uint32_t channelIndex, const float value)
		{
			if (channelIndex >= m_Channels.size())
			{
				auto msg = std::format("Tried writing to channel {} which is not a valid channel!", channelIndex);
				throw new Exception(msg);
			}

			m_Channels[channelIndex].Value = value;
		}

		ChannelValue ReadChannelValue(uint32_t channelIndex) const
		{
			if (channelIndex >= m_Channels.size())
			{
				auto msg = std::format("Tried reading from channel {} which is not a valid channel!", channelIndex);
				throw new Exception(msg);
			}

			return m_Channels[channelIndex].Value;
		}

	private:
		template<typename T>
		T* GetPrivateData() { return static_cast<T*>(m_PrivateData); }

	private:
		Type m_Type;
		std::string m_Name;
		std::string m_Manufacturer;
		std::vector<ExternalInputChannel> m_Channels;
		void* m_PrivateData;

		friend class InputDispatcher;
	};

	class InputDispatcher
	{
	public:
		InputDispatcher();
		~InputDispatcher();

		void Update();

		uint32_t GetDeviceCount() const;
		const InputDevice& GetDevice(uint32_t deviceIndex) const;
	};

}
