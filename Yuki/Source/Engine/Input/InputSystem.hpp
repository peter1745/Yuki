#pragma once

#include "Engine/Core/Unique.hpp"

#include "InputAction.hpp"
#include "InputAdapter.hpp"
#include "InputContext.hpp"

namespace Yuki {

	using InputContextID = uint32_t;

	struct ActionMetadata
	{
		InputActionID ID;
		InputContextID ContextID;
		AxisType Type;

		InputReading Reading;

		struct TriggerMetadata
		{
			Axis TargetAxis;
			const ExternalInputChannel* Channel = nullptr;
			float Scale;
		};

		std::vector<TriggerMetadata> Triggers;
	};

	class InputSystem
	{
	public:
		InputActionID RegisterAction(const InputAction& action);
		InputContextID RegisterContext(const InputContext& context);

		void ActivateContext(InputContextID contextID);
		void DeactivateContext(InputContextID contextID);

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
