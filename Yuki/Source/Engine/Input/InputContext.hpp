#pragma once

#include "InputAction.hpp"
#include "InputAxis.hpp"

#include "Engine/Core/Exception.hpp"

#include <functional>

namespace Yuki {

	class InputReading
	{
	public:
		InputReading() = default;
		InputReading(uint32_t valueCount)
		{
			m_Values.resize(valueCount, 0.0f);
		}

		void Write(uint32_t index, float32_t value)
		{
			if (index >= m_Values.size())
			{
				throw Exception("Tried writing a value into a reading that's too small");
			}

			m_Values[index] = value;
		}

		template<size_t N>
		std::array<float32_t, N> Read() const
		{
			if (N > m_Values.size())
			{
				throw Exception("Reading out of bounds");
			}

			std::array<float32_t, N> values;
			memcpy(values.data(), m_Values.data(), N * sizeof(float32_t));
			return values;
		}

	private:
		std::vector<float32_t> m_Values;
	};

	using InputActionFunction = std::function<void(const InputReading&)>;

	struct InputContext : Handle<InputContext>
	{
		void BindAction(InputAction action, InputActionFunction&& func);

		void Activate();
		void Deactivate();
	};

}
