#pragma once

#include "InputAxis.hpp"

namespace Yuki {

	struct ChannelValue
	{
		AxisValue Value;

		template<AxisValueType T>
		void Set(const T& value)
		{
			Value = value;
		}

		template<typename T>
		bool Is() const { return std::get_if<T>(&Value) != nullptr; }

		template<typename T>
		const T& ReadValue() const
		{
			const auto* value = std::get_if<T>(&Value);

			if (value == nullptr)
			{
				throw Exception("Trying to read ChannelValue incorrectly.");
			}

			return *value;
		}
	};

	struct ExternalInputChannel
	{
		ChannelValue Value;
	};

}
