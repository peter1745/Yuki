#pragma once

#include "InputAxis.hpp"

namespace Yuki {

	struct ChannelValue
	{
		AxisValue Value;
		AxisValue PreviousValue;

		template<AxisValueType T>
		void Set(const T& value)
		{
			PreviousValue = Value;
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

		bool IsDirty() const
		{
			if (const auto* value = std::get_if<AxisValue1D>(&Value.Value))
			{
				const auto& prevValue = std::get<AxisValue1D>(Value.PreviousValue);
				return value->Value != prevValue.Value;
			}
			else if (const auto* value = std::get_if<AxisValue2D>(&Value.Value))
			{
				const auto& prevValue = std::get<AxisValue2D>(Value.PreviousValue);
				return value->X != prevValue.X || value->Y != prevValue.Y;
			}
			else if (const auto* value = std::get_if<AxisValue3D>(&Value.Value))
			{
				const auto& prevValue = std::get<AxisValue3D>(Value.PreviousValue);
				return value->X != prevValue.X || value->Y != prevValue.Y || value->Z != prevValue.Z;
			}

			return false;
		}
	};

}
