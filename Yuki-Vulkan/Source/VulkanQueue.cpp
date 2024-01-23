#include "VulkanRHI.hpp"

namespace Yuki {

	Aura::Span<Queue> RHIContext::RequestQueue(QueueType type, uint32_t count) const
	{
		uint32_t startIndex = ~0;
		uint32_t endIndex = ~0;

		for (uint32_t i = 0; i < m_Impl->Queues.size(); i++)
		{
			auto queue = m_Impl->Queues[i];

			auto typeValue = std::to_underlying(type);
			if ((queue->Flags & typeValue) != typeValue)
			{
				if (startIndex != ~0)
				{
					// If we have a start index then the first non-matching queue we find is
					// one index past the last matching queue
					endIndex = i - 1;
					break;
				}

				continue;
			}

			if (startIndex == ~0)
			{
				startIndex = i;
				endIndex = i;
			}
		}

		YukiAssert(startIndex != ~0 && endIndex != ~0);

		return { &m_Impl->Queues[startIndex], endIndex - startIndex + 1 };
	}

}
