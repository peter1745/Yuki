#pragma once

#include "Engine/Common/Core.hpp"

#include "InputBindings.hpp"

namespace Yuki {

	class InputContext
	{
	public:
		template<std::derived_from<InputBinding> TBindingClass>
		void Bind(const TBindingClass& binding)
		{
			m_Bindings.emplace_back(std::move(Unique<TBindingClass>::New(binding)));
		}

		template<std::derived_from<InputEvent> TEventClass>
		void ProcessInput(const TEventClass& event) const
		{
			for (const auto& binding : m_Bindings)
			{
				if (binding->TryHandle(&event))
					break;
			}
		}

	private:
		DynamicArray<Unique<InputBinding>> m_Bindings;
	};

}
