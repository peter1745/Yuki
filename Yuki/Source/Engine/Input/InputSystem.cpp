#include "InputSystem.hpp"

#include <ranges>

namespace Yuki {

	InputActionID InputSystem::RegisterAction(const InputAction& action)
	{
		InputActionID id = m_Actions.size();
		m_Actions.push_back(action);
		return id;
	}

	InputContextID InputSystem::RegisterContext(const InputContext& context)
	{
		InputContextID id = m_Contexts.size();
		m_Contexts.push_back(context); // TODO: std::move
		return id;
	}

	void InputSystem::ActivateContext(InputContextID contextID)
	{
		m_Contexts[contextID].Active = true;

		for (uint32_t i = 0; i < m_Contexts.size(); i++)
		{
			const auto& context = m_Contexts[i];

			if (!context.Active)
				continue;

			for (const auto& [actionID, actionFunc] : context.m_ActionBindings)
			{
				const auto& action = m_Actions[actionID];

				auto& actionMetadata = m_ActionMetadata.emplace_back();
				actionMetadata.ID = actionID;
				actionMetadata.ContextID = i;
				actionMetadata.Type = action.Type;

				for (const auto& axisBinding : action.AxisBindings)
				{
					for (const auto& triggerBinding : axisBinding.Bindings)
					{
						const auto& device = m_Adapter.GetDevice(triggerBinding.ID.DeviceID);
						auto& actionTrigger = actionMetadata.Triggers.emplace_back();
						actionTrigger.TargetAxis = axisBinding.TargetAxis;
						actionTrigger.Channel = device.GetChannel(triggerBinding.ID.InputID);
						actionTrigger.Scale = triggerBinding.Scale;
					}
				}
			}
		}
	}

	void InputSystem::DeactivateContext(InputContextID contextID)
	{
		m_Contexts[contextID].Active = false;
	}

	void InputSystem::Update()
	{
		m_Adapter.Update();

		for (const auto& actionMetadata : m_ActionMetadata)
		{
			bool triggered = false;

			InputReading reading(actionMetadata.Type);

			for (const auto& trigger : actionMetadata.Triggers)
			{
				if (!trigger.Channel->IsDirty())
					continue;

				reading.Write(trigger.TargetAxis, trigger.Channel->Value.ReadValue<AxisValue1D>().Value * trigger.Scale);
				triggered = true;
			}

			if (!triggered)
				continue;

			m_Contexts[actionMetadata.ContextID].m_ActionBindings[actionMetadata.ID](reading);
		}
	}

}
