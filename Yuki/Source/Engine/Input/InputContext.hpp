#pragma once

#include "Engine/Common/Core.hpp"

#include "InputBindings.hpp"

namespace Yuki {

	class InputContext
	{
	public:
		template<std::derived_from<InputBinding> TBindingClass>
		void Bind(const TBindingClass& InBinding)
		{
			m_Bindings.emplace_back(std::move(Unique<TBindingClass>::New(InBinding)));
		}

		template<std::derived_from<InputEvent> TEventClass>
		void ProcessInput(const TEventClass& InEvent) const
		{
			for (const auto& Binding : m_Bindings)
			{
				if (Binding->TryHandle(&InEvent))
					break;
			}
		}

	private:
		DynamicArray<Unique<InputBinding>> m_Bindings;
	};

}
