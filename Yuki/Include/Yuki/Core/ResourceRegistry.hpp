#pragma once

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

		std::shared_mutex Mutex;
		std::vector<ElementFlag> Flags;
		std::vector<TKey> FreeList;
		std::vector<TElement> Elements;

		ResourceRegistry()
		{
			std::scoped_lock lock(Mutex);
			Flags.push_back(ElementFlag::Empty);
			Elements.push_back({});
		}

		std::pair<TKey, TElement&> Acquire()
		{
			std::scoped_lock lock(Mutex);

			if (FreeList.empty())
			{
				auto& element = Elements.emplace_back();
				Flags.emplace_back(ElementFlag::Exists);
				return { TKey(Elements.size() - 1), element };
			}

			TKey key = FreeList.back();
			FreeList.pop_back();
			Flags[static_cast<std::underlying_type_t<TKey>>(key)] = ElementFlag::Exists;
			return { key, Elements[static_cast<std::underlying_type_t<TKey>>(key)] };
		}

		void Return(TKey InKey)
		{
			std::scoped_lock lock(Mutex);
			Flags[static_cast<std::underlying_type_t<TKey>>(InKey)] = ElementFlag::Empty;
			FreeList.emplace_back(InKey);
		}

		bool IsValid(TKey InKey) const
		{
			//std::scoped_lock lock(Mutex);
			return Flags[static_cast<std::underlying_type_t<TKey>>(InKey)] == ElementFlag::Exists;
		}

		TElement& Get(TKey InKey)
		{
			std::scoped_lock lock(Mutex);
			return Elements[static_cast<std::underlying_type_t<TKey>>(InKey)];
		}

		const TElement& Get(TKey InKey) const
		{
			//std::scoped_lock lock(Mutex);
			return Elements[static_cast<std::underlying_type_t<TKey>>(InKey)];
		}

		template<typename TFunction>
		void ForEach(TFunction&& InFunction)
		{
			std::scoped_lock lock(Mutex);
			for (size_t i = 0; i < Elements.size(); i++)
			{
				if (Flags[i] == ElementFlag::Exists)
					InFunction(TKey(i), Elements[i]);
			}
		}

		template<typename TFunction>
		void ForEach(TFunction&& InFunction) const
		{
			//std::scoped_lock lock(Mutex);
			for (size_t i = 0; i < Elements.size(); i++)
			{
				if (Flags[i] == ElementFlag::Exists)
					InFunction(TKey(i), Elements[i]);
			}
		}

		uint32_t GetCount() const
		{
			//std::scoped_lock lock(Mutex);
			return uint32_t(Elements.size() - FreeList.size());
		}
	};

}
