#pragma once

#include <vector>
#include <utility>
#include <type_traits>

namespace Yuki {
	
	template<typename TKey, typename TElement>
	struct ResourceRegistry
	{
		enum class ElementFlag : uint8_t
		{
			Empty = 0,
			Exists = 1,
		};

		std::vector<ElementFlag> Flags;
		std::vector<TKey> FreeList;
		std::vector<TElement> Elements;

		ResourceRegistry()
		{
			Flags.push_back(ElementFlag::Empty);
			Elements.push_back({});
		}

		std::pair<TKey, TElement&> Acquire()
		{
			if (FreeList.empty())
			{
				auto& element = Elements.emplace_back();
				Flags.emplace_back(ElementFlag::Exists);
				return { TKey(Elements.size() - 1), element };
			}

			TKey key = FreeList.back();
			FreeList.pop_back();
			Flags[static_cast<std::underlying_type_t<TKey>>(key)] = ElementFlag::Exists;
			return { key, Get(key) };
		}

		void Return(TKey InKey)
		{
			Flags[static_cast<std::underlying_type_t<TKey>>(InKey)] = ElementFlag::Empty;
			FreeList.emplace_back(InKey);
		}

		bool IsValid(TKey InKey) const
		{
			return Flags[static_cast<std::underlying_type_t<TKey>>(InKey)] == ElementFlag::Exists;
		}

		TElement& Get(TKey InKey) { return Elements[static_cast<std::underlying_type_t<TKey>>(InKey)]; }
		const TElement& Get(TKey InKey) const { return Elements[static_cast<std::underlying_type_t<TKey>>(InKey)]; }

		template<typename TFunction>
		void ForEach(TFunction&& InFunction)
		{
			for (size_t i = 0; i < Elements.size(); i++)
			{
				if (Flags[i] == ElementFlag::Exists)
					InFunction(TKey(i), Elements[i]);
			}
		}

		template<typename TFunction>
		void ForEach(TFunction&& InFunction) const
		{
			for (size_t i = 0; i < Elements.size(); i++)
			{
				if (Flags[i] == ElementFlag::Exists)
					InFunction(TKey(i), Elements[i]);
			}
		}

		uint32_t GetCount() const { return uint32_t(Elements.size() - FreeList.size()); }
	};

}
