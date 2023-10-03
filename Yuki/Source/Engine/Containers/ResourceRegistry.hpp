#pragma once

#include "StableList.hpp"

#include <utility>
#include <type_traits>

namespace Yuki {

	template<typename TKey, typename TElement>
	class ResourceRegistry
	{
	public:
		ResourceRegistry()
		{
			std::scoped_lock Lock(m_Mutex);
			auto [Index, Entry] = m_Elements.EmplaceBackNoLock();
			Entry.first = ElementFlag::Exists;
		}

		std::pair<TKey, TElement&> Acquire(bool InMarkReady = true)
		{
			std::scoped_lock Lock(m_Mutex);

			if (m_FreeList.empty())
			{
				auto [Index, Entry] = m_Elements.EmplaceBackNoLock();
				Entry.first = InMarkReady ? ElementFlag::Exists : ElementFlag::Empty;
				return { TKey(Index), Entry.second };
			}

			TKey Key = m_FreeList.back();
			m_FreeList.pop_back();
			auto& [Flag, Element] = m_Elements[static_cast<std::underlying_type_t<TKey>>(Key)];
			Flag = InMarkReady ? ElementFlag::Exists : ElementFlag::Empty;
			return { Key, Element };
		}

		std::pair<TKey, TElement&> Insert(const TElement& InElement, bool InMarkReady = false)
		{
			std::scoped_lock Lock(m_Mutex);

			if (m_FreeList.empty())
			{
				auto [Index, Entry] = m_Elements.EmplaceBackNoLock();
				Entry.first = InMarkReady ? ElementFlag::Exists : ElementFlag::Empty;
				Entry.second = InElement;
				return { TKey(Index), Entry.second };
			}

			TKey Key = m_FreeList.back();
			m_FreeList.pop_back();
			auto& [Flag, Element] = m_Elements[static_cast<std::underlying_type_t<TKey>>(Key)];
			Flag = InMarkReady ? ElementFlag::Exists : ElementFlag::Empty;
			Element = InElement;
			return { Key, Element };
		}

		void MarkReady(TKey InKey)
		{
			auto& [Flag, Element] = m_Elements[static_cast<std::underlying_type_t<TKey>>(InKey)];
			Flag = ElementFlag::Exists;
		}

		void Return(TKey InKey)
		{
			std::scoped_lock Lock(m_Mutex);
			auto& [Flag, Element] = m_Elements[static_cast<std::underlying_type_t<TKey>>(InKey)];
			Flag = ElementFlag::Empty;
			m_FreeList.emplace_back(InKey);
		}

		bool IsValid(TKey InKey) const
		{
			const auto& [Flag, Element] = m_Elements[static_cast<std::underlying_type_t<TKey>>(InKey)];
			return Flag == ElementFlag::Exists;
		}

		TElement& Get(TKey InKey)
		{
			return m_Elements[static_cast<std::underlying_type_t<TKey>>(InKey)].second;
		}

		const TElement& Get(TKey InKey) const
		{
			return m_Elements[static_cast<std::underlying_type_t<TKey>>(InKey)].second;
		}

		TElement& operator[](TKey InKey) { return Get(InKey); }
		const TElement& operator[](TKey InKey) const { return Get(InKey); }

		template<typename TFunction>
		void ForEach(TFunction&& InFunction)
		{
			for (size_t Index = 1; Index < m_Elements.GetCount(); Index++)
			{
				auto& [Flag, Element] = m_Elements[Index];
				if (Flag == ElementFlag::Exists)
					InFunction(TKey(Index), Element);
			}
		}

		template<typename TFunction>
		void ForEach(TFunction&& InFunction) const
		{
			for (size_t Index = 1; Index < m_Elements.GetCount(); Index++)
			{
				const auto& [Flag, Element] = m_Elements[Index];
				if (Flag == ElementFlag::Exists)
					InFunction(TKey(Index), Element);
			}
		}

		uint32_t GetCount() const
		{
			std::scoped_lock lock(m_Mutex);
			return uint32_t(m_Elements.GetCount() - m_FreeList.size());
		}

	private:
		enum class ElementFlag : uint8_t
		{
			Empty = 0,
			Exists = 1,
		};

	private:
		mutable std::shared_mutex m_Mutex;
		std::vector<TKey> m_FreeList;
		StableList<std::pair<std::atomic<ElementFlag>, TElement>> m_Elements;
	};

}
