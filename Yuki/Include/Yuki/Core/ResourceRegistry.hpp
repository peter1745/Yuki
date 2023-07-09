#pragma once

#include "Yuki/Core/StableDynamicArray.hpp"

#include <vector>
#include <utility>
#include <type_traits>
#include <shared_mutex>

namespace Yuki {
	
	template<typename TKey, typename TElement>
	struct ResourceRegistry
	{
		enum class ElementFlag : uint8_t
		{
			Empty = 0,
			Exists = 1,
		};

		mutable std::shared_mutex Mutex;
		std::vector<TKey> FreeList;
		StableDynamicArray<std::pair<std::atomic<ElementFlag>, TElement>> Elements;

		ResourceRegistry()
		{
			std::scoped_lock lock(Mutex);
			auto[index, entry] = Elements.EmplaceBackNoLock();
			entry.first = ElementFlag::Exists;
		}

		std::pair<TKey, TElement&> Acquire(bool InMarkReady = true)
		{
			std::scoped_lock lock(Mutex);

			if (FreeList.empty())
			{
				auto[index, entry] = Elements.EmplaceBackNoLock();
				entry.first = InMarkReady ? ElementFlag::Exists : ElementFlag::Empty;
				return { TKey(index), entry.second };
			}

			TKey key = FreeList.back();
			FreeList.pop_back();
			auto&[flag, element] = Elements[static_cast<std::underlying_type_t<TKey>>(key)];
			flag = InMarkReady ? ElementFlag::Exists : ElementFlag::Empty;
			return { key, element };
		}

		std::pair<TKey, TElement&> Insert(const TElement& InElement, bool InMarkReady = false)
		{
			std::scoped_lock lock(Mutex);

			if (FreeList.empty())
			{
				auto[index, entry] = Elements.EmplaceBackNoLock();
				entry.first = InMarkReady ? ElementFlag::Exists : ElementFlag::Empty;
				entry.second = InElement;
				return { TKey(index), entry.second };
			}

			TKey key = FreeList.back();
			FreeList.pop_back();
			auto&[flag, element] = Elements[static_cast<std::underlying_type_t<TKey>>(key)];
			flag = InMarkReady ? ElementFlag::Exists : ElementFlag::Empty;
			element = InElement;
			return { key, element };
		}

		void MarkReady(TKey InKey)
		{
			auto&[flag, element] = Elements[static_cast<std::underlying_type_t<TKey>>(InKey)];
			flag = ElementFlag::Exists;
		}

		void Return(TKey InKey)
		{
			std::scoped_lock lock(Mutex);
			auto&[flag, element] = Elements[static_cast<std::underlying_type_t<TKey>>(InKey)];
			flag = ElementFlag::Empty;
			FreeList.emplace_back(InKey);
		}

		bool IsValid(TKey InKey) const
		{
			const auto&[flag, element] = Elements[static_cast<std::underlying_type_t<TKey>>(InKey)];
			return flag == ElementFlag::Exists;
		}

		TElement& Get(TKey InKey)
		{
			return Elements[static_cast<std::underlying_type_t<TKey>>(InKey)].second;
		}

		const TElement& Get(TKey InKey) const
		{
			return Elements[static_cast<std::underlying_type_t<TKey>>(InKey)].second;
		}

		template<typename TFunction>
		void ForEach(TFunction&& InFunction)
		{
			for (size_t i = 1; i < Elements.GetElementCount(); i++)
			{
				auto&[flag, element] = Elements[i];
				if (flag == ElementFlag::Exists)
					InFunction(TKey(i), element);
			}
		}

		template<typename TFunction>
		void ForEach(TFunction&& InFunction) const
		{
			for (size_t i = 1; i < Elements.GetElementCount(); i++)
			{
				const auto&[flag, element] = Elements[i];
				if (flag == ElementFlag::Exists)
					InFunction(TKey(i), element);
			}
		}

		uint32_t GetCount() const
		{
			std::scoped_lock lock(Mutex);
			return uint32_t(Elements.GetElementCount() - FreeList.size());
		}
	};

}
