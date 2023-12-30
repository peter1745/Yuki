#pragma once

#include "InputAxis.hpp"

#include "Engine/Core/Exception.hpp"

#include <functional>

namespace Yuki {

	using InputActionID = uint32_t;

	class InputReading
	{
	public:
		template<AxisValueType T>
		const T& Read() const
		{
			const T* value = std::get_if<T>(&m_Value);

			if (value == nullptr)
			{
				throw Exception("Invalid read ya dingus");
			}

			return *value;
		}

	private:
		InputReading(AxisType type)
			: m_Type(type)
		{
			switch (type)
			{
			case AxisType::Axis1D:
			{
				m_Value = AxisValue1D{};
				break;
			}
			case AxisType::Axis2D:
			{
				m_Value = AxisValue2D{};
				break;
			}
			case AxisType::Axis3D:
			{
				m_Value = AxisValue3D{};
				break;
			}
			}
		}

		void Write(Axis axis, float value)
		{
			if (std::to_underlying(axis) > std::to_underlying(m_Type))
			{
				throw Exception("Tried writing a value into a reading that's too small");
			}

			switch (m_Type)
			{
			case AxisType::Axis1D:
			{
				std::get<AxisValue1D>(m_Value).Value = value;
				break;
			}
			case AxisType::Axis2D:
			{
				auto& axisValue = std::get<AxisValue2D>(m_Value);

				if (axis == Axis::X)
					axisValue.X = value;
				else if (axis == Axis::Y)
					axisValue.Y = value;
				break;
			}
			case AxisType::Axis3D:
			{
				auto& axisValue = std::get<AxisValue3D>(m_Value);

				if (axis == Axis::X)
					axisValue.X = value;
				else if (axis == Axis::Y)
					axisValue.Y = value;
				else if (axis == Axis::Y)
					axisValue.Z = value;
				break;
			}
			}
		}

	private:
		AxisType m_Type;
		AxisValue m_Value;

		friend class InputSystem;
	};

	class InputContext
	{
		using ActionFunction = std::function<void(InputReading)>;
	public:
		void BindAction(InputActionID actionID, ActionFunction func);

	public:
		bool Active = false;

	private:
		std::unordered_map<InputActionID, ActionFunction> m_ActionBindings;

		friend class InputSystem;
	};

}
