#pragma once

#include "Engine/Core/Exception.hpp"

#include "ExternalInputChannel.hpp"

namespace Yuki {

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

		InputDevice(Type type, std::string_view name, std::string_view manufacturer, void* privateData)
			: m_Type(type), m_Name(name), m_Manufacturer(manufacturer), m_PrivateData(privateData)
		{
		}

		Type GetType() const { return m_Type; }
		std::string_view GetName() const { return m_Name; }
		std::string_view GetManufacturer() const { return m_Manufacturer; }

		template<AxisValueType T>
		void RegisterChannel()
		{
			m_Channels.emplace_back(ChannelValue{ .Value = T{} });
		}

		template<AxisValueType T>
		void WriteChannelValue(uint32_t channelIndex, const T& value)
		{
			if (channelIndex >= m_Channels.size())
			{
				auto msg = std::format("Tried writing to channel {} which is not a valid channel!", channelIndex);
				throw Exception(msg);
			}

			auto& channelValue = m_Channels[channelIndex].Value;

			if (!channelValue.Is<T>())
			{
				throw Exception("Trying to write a value to a channel of the wrong type.");
			}

			channelValue.Set(value);
		}

		const ChannelValue& ReadChannelValue(uint32_t channelIndex) const
		{
			if (channelIndex >= m_Channels.size())
			{
				auto msg = std::format("Tried reading from channel {} which is not a valid channel!", channelIndex);
				throw Exception(msg);
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
