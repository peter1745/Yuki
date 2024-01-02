#pragma once

#include "Engine/Core/Unique.hpp"

#include "InputAction.hpp"
#include "InputAdapter.hpp"
#include "InputContext.hpp"

namespace Yuki {

	struct ActionMetadata
	{
		InputAction Action;
		InputContext Context;

		InputReading Reading;

		struct TriggerMetadata
		{
			uint32_t AxisIndex;
			const ExternalInputChannel* Channel = nullptr;
			float32_t Scale;
		};

		std::vector<TriggerMetadata> Triggers;
	};

	class InputSystem
	{
	public:
		InputSystem();
		~InputSystem();

		InputAction RegisterAction(const InputActionData& actionData);
		InputContext CreateContext();

	private:
		void Update();

		void GenerateActionMetadata();

	private:
		InputAdapter m_Adapter;

		std::vector<InputAction> m_Actions;
		std::vector<InputContext> m_Contexts;

		std::vector<ActionMetadata> m_ActionMetadata;

		friend class Application;
	};

}
