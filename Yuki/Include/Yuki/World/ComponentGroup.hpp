#pragma once

#include <flecs/flecs.h>

namespace Yuki {

	template<typename... TComponents>
	struct ComponentGroup {};

	namespace ComponentIter {
		
		template<typename TFunc, typename... TComponents>
		inline void Each(flecs::entity InEntity, TFunc InFunc)
		{
			([&]()
			{
				InFunc(InEntity.get_mut<TComponents>());
			}(), ...);
		}

		template<typename TFunc, typename... TComponents>
		inline void EachConst(flecs::entity InEntity, TFunc InFunc)
		{
			([&]()
			{
				InFunc(InEntity.get<TComponents>());
			}(), ...);
		}

		template<typename TFunc, typename... TComponents>
		inline void Each(flecs::entity InEntity, ComponentGroup<TComponents...> InComponentGroup, TFunc InFunc)
		{
			Each<TFunc, TComponents...>(InEntity, InFunc);
		}

		template<typename TFunc, typename... TComponents>
		inline void EachConst(flecs::entity InEntity, ComponentGroup<TComponents...> InComponentGroup, TFunc InFunc)
		{
			EachConst<TFunc, TComponents...>(InEntity, InFunc);
		}

	}

}
