#pragma once

#include "Yuki/Core/Core.hpp"
#include "Yuki/Core/Debug.hpp"

namespace Yuki {

	class Buffer
	{
	public:
		Buffer(size_t InSize)
			: m_Size(InSize)
		{
			m_Data = new std::byte[InSize];
		}

		~Buffer()
		{
			delete[] m_Data;
		}

		template<typename T>
		void Write(T InValue)
		{
			YUKI_VERIFY(m_WriteOffset + sizeof(InValue) <= m_Size);
			memcpy(m_Data + m_WriteOffset, &InValue, sizeof(InValue));
			m_WriteOffset += sizeof(InValue);
		}

		template<>
		void Write(std::string_view InString)
		{
			YUKI_VERIFY(m_WriteOffset + InString.size() <= m_Size);
			memcpy(m_Data + m_WriteOffset, InString.data(), InString.size());
			m_WriteOffset += InString.size();
		}

		template<typename T>
		void WriteArray(const DynamicArray<T>& InValues)
		{
			for (const auto& value : InValues)
				Write<T>(value);
		}

		const std::byte* Data() const { return m_Data; }

	private:
		std::byte* m_Data = nullptr;
		size_t m_Size = 0;
		size_t m_WriteOffset = 0;
	};

}