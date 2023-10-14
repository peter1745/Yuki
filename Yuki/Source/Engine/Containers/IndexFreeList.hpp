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

}
