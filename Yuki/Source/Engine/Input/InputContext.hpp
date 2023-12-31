#pragma once

#include "InputAxis.hpp"

#include "Engine/Core/Exception.hpp"

#include <functional>

namespace Yuki {

	using InputActionID = uint32_t;

	class InputReading
	{
	public:
		InputReading() = default;

		template<size_t N>
		std::array<float, N> Read() const
		{
			if (N > m_Values.size())
			{
				throw Exception("Reading out of bounds");
			}

			std::array<float, N> values;
			memcpy(values.data(), m_Values.data(), N * sizeof(float));
			return values;
		}

	private:
		InputReading(uint32_t valueCount)
		{
			m_Values.resize(valueCount, 0.0f);
		}

		void Write(uint32_t index, float value)
		{
			if (index >= m_Values.size())
			{
				throw Exception("Tried writing a value into a reading that's too small");
			}

			m_Values[index] = value;
		}

	private:
		std::vector<float> m_Values;

		friend class InputSystem;
	};

	using InputActionFunction = std::function<void(const InputReading&)>;

	class InputContext
	{
	public:
		bool Active = false;

	private:
		void InvokeActionFunction(InputActionID actionID, const InputReading& reading)
		{
			if (!m_ActionBindings.contains(actionID))
				return;

			m_ActionBindings.at(actionID)(reading);
		}

	private:
		std::unordered_map<InputActionID, InputActionFunction> m_ActionBindings;

		friend class InputSystem;
	};

}
