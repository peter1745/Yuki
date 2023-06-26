#pragma once

#include <vector>
#include <array>
#include <shared_mutex>

namespace Yuki {

	template<typename TElement, size_t PageSize = 256>
	class StableDynamicArray
	{
		struct Page
		{
			std::array<TElement, PageSize> Elements;
		};

		std::shared_mutex m_Mutex;
		std::list<std::unique_ptr<Page*[]>> m_PageTables;

		std::atomic<Page**> m_Pages;
		std::atomic<uint32_t> m_ElementCount = 0;
		std::atomic<uint32_t> m_Capacity = 0;
		size_t m_PageCount = 0;

	public:
		TElement& operator[](size_t index)
		{
			size_t pageIndex = index / PageSize;
			return m_Pages[pageIndex]->Elements[index - (pageIndex * PageSize)];
		}

		const TElement& operator[](size_t index) const
		{
			size_t pageIndex = index / PageSize;
			return m_Pages[pageIndex]->Elements[index - (pageIndex * PageSize)];
		}

		size_t GetElementCount() const { return m_ElementCount; }

		std::pair<uint32_t, TElement&> EmplaceBack()
		{
			size_t pageIndex = m_ElementCount / PageSize;

			if (m_ElementCount >= m_Capacity)
			{
				std::scoped_lock lock(m_Mutex);

				if (m_ElementCount >= m_Capacity)
				{
					auto* newPage = new Page();
					
					if (pageIndex >= m_PageCount)
					{
						auto oldPages = m_PageCount;
						m_PageCount = std::max(16ull, m_PageCount * 2);
						auto newPageTable = std::make_unique<Page*[]>(m_PageCount);
						std::memcpy(newPageTable.get(), m_Pages.load(), oldPages * sizeof(void*));
						m_Pages.exchange(newPageTable.get());
						m_PageTables.push_back(std::move(newPageTable));
					}

					m_Pages[pageIndex] = newPage;

					m_Capacity += PageSize;
				}
			}

			uint32_t index = (++m_ElementCount - 1);
			return { index, m_Pages[pageIndex]->Elements[index - (pageIndex * PageSize)] };
		}

		std::pair<uint32_t, TElement&> EmplaceBackNoLock()
		{
			size_t pageIndex = m_ElementCount / PageSize;

			if (m_ElementCount >= m_Capacity)
			{
				auto* newPage = new Page();
				
				if (pageIndex >= m_PageCount)
				{
					auto oldPages = m_PageCount;
					m_PageCount = std::max(16ull, m_PageCount * 2);
					auto newPageTable = std::make_unique<Page*[]>(m_PageCount);
					std::memcpy(newPageTable.get(), m_Pages.load(), oldPages * sizeof(void*));
					m_Pages.exchange(newPageTable.get());
					m_PageTables.push_back(std::move(newPageTable));
				}

				m_Pages[pageIndex] = newPage;

				m_Capacity += PageSize;
			}

			uint32_t index = (++m_ElementCount - 1);
			return { index, m_Pages[pageIndex]->Elements[index - (pageIndex * PageSize)] };
		}

		template<typename Fn>
		void ForEach(Fn&& fn)
		{
			for (uint32_t i = 0; i < m_ElementCount; ++i)
				fn((*this)[i]);
		}
	};

}
