#include "InputSystem.hpp"

#include <ranges>

namespace Yuki {

	InputActionID InputSystem::RegisterAction(const InputAction& action)
	{
		InputActionID id = static_cast<InputActionID>(m_Actions.size());
		m_Actions.push_back(action);
		return id;
	}

	InputContextID InputSystem::RegisterContext(const InputContext& context)
	{
		InputContextID id = static_cast<InputContextID>(m_Contexts.size());
		m_Contexts.push_back(context); // TODO: std::move
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

		for (auto& actionMetadata : m_ActionMetadata)
		{
			bool triggered = false;

			for (const auto& trigger : actionMetadata.Triggers)
			{
				if (!trigger.Channel->IsDirty())
					continue;

				actionMetadata.Reading.Write(trigger.Index, trigger.Channel->Value * trigger.Scale);
				triggered = true;
			}

			if (!triggered)
				continue;

			m_Contexts[actionMetadata.ContextID].m_ActionBindings[actionMetadata.ID](actionMetadata.Reading);
		}
	}

	void InputSystem::GenerateActionMetadata()
	{
		m_ActionMetadata.clear();

		std::vector<const ExternalInputChannel*> consumedChannels;

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
				actionMetadata.Reading = InputReading(action.ValueCount);

				for (uint32_t j = 0; j < action.AxisBindings.size(); j++)
				{
					const auto& axisBinding = action.AxisBindings[j];

					for (const auto& triggerBinding : axisBinding.Bindings)
					{
						const auto& device = m_Adapter.GetDevice(triggerBinding.ID.DeviceID);
						const auto* channel = device.GetChannel(triggerBinding.ID.InputID);

						// If our channel has been marked as consumed already we just ignore this trigger (other triggers may still work)
						if (std::ranges::find(consumedChannels, channel) != consumedChannels.end())
							continue;

						auto& actionTrigger = actionMetadata.Triggers.emplace_back();
						actionTrigger.Index = j;
						actionTrigger.Channel = channel;
						actionTrigger.Scale = triggerBinding.Scale;

						if (action.ConsumeInputs)
						{
							consumedChannels.push_back(channel);
						}
					}
				}
			}
		}
	}

}
