#include "InputSystem.hpp"

#include <ranges>

namespace Yuki {

	InputActionID InputSystem::RegisterAction(const InputAction& action)
	{
		InputActionID id = static_cast<InputActionID>(m_Actions.size());
		m_Actions.push_back(action);
		return id;
	}

	InputContextID InputSystem::CreateContext()
	{
		InputContextID id = static_cast<InputContextID>(m_Contexts.size());
		m_Contexts.push_back({});
		return id;
	}

	void InputSystem::BindAction(InputContextID contextID, InputActionID actionID, InputActionFunction func)
	{
		auto& context = m_Contexts[contextID];
		context.m_ActionBindings[actionID] = std::move(func);

		if (context.Active)
		{
			GenerateActionMetadata();
		}
	}

	void InputSystem::ActivateContext(InputContextID contextID)
	{
		m_Contexts[contextID].Active = true;
		GenerateActionMetadata();
	}

	void InputSystem::DeactivateContext(InputContextID contextID)
	{
		m_Contexts[contextID].Active = false;
		GenerateActionMetadata();
	}

	void InputSystem::Update()
	{
		m_Adapter.Update();

		// Dispatch input events to the active actions
		for (auto& actionMetadata : m_ActionMetadata)
		{
			bool triggered = false;

			for (const auto& trigger : actionMetadata.Triggers)
			{
				// If the channel hasn't changed we don't need to do anything (this will require more complex behavior in the future)
				if (!trigger.Channel->IsDirty())
					continue;

				actionMetadata.Reading.Write(trigger.AxisIndex, trigger.Channel->Value * trigger.Scale);
				triggered = true;
			}

			if (!triggered)
				continue;

			// Dispatch the reading to the bound action
			m_Contexts[actionMetadata.ContextID].InvokeActionFunction(actionMetadata.ID, actionMetadata.Reading);
		}
	}

	void InputSystem::GenerateActionMetadata()
	{
		m_ActionMetadata.clear();

		std::vector<const ExternalInputChannel*> consumedChannels;

		for (uint32_t contextIndex = 0; contextIndex < m_Contexts.size(); contextIndex++)
		{
			const auto& context = m_Contexts[contextIndex];

			if (!context.Active)
				continue;

			for (const auto& [actionID, actionFunc] : context.m_ActionBindings)
			{
				const auto& action = m_Actions[actionID];

				auto& actionMetadata = m_ActionMetadata.emplace_back();
				actionMetadata.ID = actionID;
				actionMetadata.ContextID = contextIndex;
				actionMetadata.Reading = InputReading(action.ValueCount);

				for (uint32_t axisIndex = 0; axisIndex < action.AxisBindings.size(); axisIndex++)
				{
					const auto& axisBinding = action.AxisBindings[axisIndex];

					for (const auto& triggerBinding : axisBinding.Bindings)
					{
						const auto* device = m_Adapter.GetDevice(triggerBinding.ID.DeviceID);

						if (device == nullptr)
							continue;

						const auto* channel = device->GetChannel(triggerBinding.ID.InputID);

						if (channel == nullptr)
							continue;

						// If our channel has been marked as consumed already we just ignore this trigger (other triggers may still work)
						if (std::ranges::find(consumedChannels, channel) != consumedChannels.end())
							continue;

						auto& actionTrigger = actionMetadata.Triggers.emplace_back();
						actionTrigger.AxisIndex = axisIndex;
						actionTrigger.Channel = channel;
						actionTrigger.Scale = triggerBinding.Scale;

						if (action.ConsumeInputs)
						{
							// Make sure that no other trigger bindings can use our channel if we consume it
							consumedChannels.push_back(channel);
						}
					}
				}
			}
		}
	}

}
