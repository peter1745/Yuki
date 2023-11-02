#pragma once

#include "Engine/Common/Types.hpp"

namespace Yuki {

	class IndexFreeList
	{
	public:
		uint32_t Acquire()
		{
			if (m_FreeList.empty())
				return m_NextIndex++;

			auto index = m_FreeList.back();
			m_FreeList.pop_back();
			return index;
		}

		void Return(uint32_t index)
		{
			m_FreeList.push_back(index);
		}

	private:
		uint32_t m_NextIndex = 0;
		DynamicArray<uint32_t> m_FreeList;
	};

	class FixedIndexFreeList
	{
	public:
		FixedIndexFreeList(uint32_t maxIndex)
			: m_MaxIndex(maxIndex)
		{
			m_FreeList.resize(maxIndex);
			for (uint32_t i = 0; i < maxIndex; i++)
				m_FreeList[i] = true;
		}

		bool IsFree(uint32_t index) const { return m_FreeList[index]; }

		void Acquire(uint32_t index)
		{
			m_FreeList[index] = false;
		}

		void Return(uint32_t index)
		{
			m_FreeList[index] = true;
		}

	private:
		uint32_t m_MaxIndex;
		DynamicArray<bool> m_FreeList;
	};

}
