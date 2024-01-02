#include "InputSystem.hpp"

#include <ranges>

namespace Yuki {

	template<>
	struct Handle<InputContext>::Impl
	{
		bool IsDirty = false;
		bool IsActive = false;

		std::vector<InputAction> Actions;
		std::unordered_map<InputAction::ID, InputActionFunction> ActionFunctions;

		void InvokeAction(InputAction action, const InputReading& reading)
		{
			ActionFunctions[action.GetID()](reading);
		}
	};

	template<>
	struct Handle<InputAction>::Impl
	{
		InputActionData Data;
	};

	void InputContext::Activate()
	{
		m_Impl->IsActive = true;
		m_Impl->IsDirty = true;
	}

	void InputContext::Deactivate()
	{
		m_Impl->IsActive = false;
		m_Impl->IsDirty = true;
	}

	void InputContext::BindAction(InputAction action, InputActionFunction&& func)
	{
		m_Impl->Actions.push_back(action);
		m_Impl->ActionFunctions[action.GetID()] = std::move(func);
		m_Impl->IsDirty = true;
	}

	InputSystem::InputSystem()
	{
		m_Adapter = InputAdapter::Create();
	}

	InputSystem::~InputSystem()
	{
		m_Adapter.Destroy();
	}

	InputAction InputSystem::RegisterAction(const InputActionData& actionData)
	{
		auto* action = new InputAction::Impl();
		action->Data = actionData;
		m_Actions.push_back({ action });
		return { action };
	}

	InputContext InputSystem::CreateContext()
	{
		auto* contextImpl = new InputContext::Impl();
		m_Contexts.push_back({ contextImpl });
		return { contextImpl };
	}

	void InputSystem::Update()
	{
		m_Adapter.Update();

		// Rebuild action metadata if necessary
		for (auto context : m_Contexts)
		{
			if (context->IsActive && context->IsDirty)
			{
				GenerateActionMetadata();
				break;
			}
		}

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
			actionMetadata.Context->InvokeAction(actionMetadata.Action, actionMetadata.Reading);
		}
	}

	void InputSystem::GenerateActionMetadata()
	{
		m_ActionMetadata.clear();

		std::unordered_map<InputAction::ID, const ExternalInputChannel*> consumedChannels;

		for (auto context : m_Contexts)
		{
			if (!context->IsActive)
				continue;

			for (auto action : context->Actions)
			{
				auto& actionMetadata = m_ActionMetadata.emplace_back();
				actionMetadata.Action = action;
				actionMetadata.Context = context;
				actionMetadata.Reading = InputReading(action->Data.ValueCount);

				for (uint32_t axisIndex = 0; axisIndex < action->Data.AxisBindings.size(); axisIndex++)
				{
					const auto& axisBinding = action->Data.AxisBindings[axisIndex];

					for (const auto& triggerBinding : axisBinding.Bindings)
					{
						const auto device = m_Adapter.GetDevice(triggerBinding.ID.DeviceID);

						if (!device)
							continue;

						const auto* channel = device.GetChannel(triggerBinding.ID.InputID);

						if (channel == nullptr)
							continue;

						// If our channel has been marked as consumed already we just ignore this trigger (other triggers may still work)
						bool usesConsumedChanel = false;
						for (auto [otherActionID, consumedChannel] : consumedChannels)
						{
							if (otherActionID != action.GetID() && channel == consumedChannel)
							{
								usesConsumedChanel = true;
								break;
							}
						}

						if (usesConsumedChanel)
							continue;

						auto& actionTrigger = actionMetadata.Triggers.emplace_back();
						actionTrigger.AxisIndex = axisIndex;
						actionTrigger.Channel = channel;
						actionTrigger.Scale = triggerBinding.Scale;

						if (action->Data.ConsumeInputs)
						{
							// Make sure that no other trigger bindings can use our channel if we consume it
							consumedChannels[action.GetID()] = channel;
						}
					}
				}
			}
		}

		// Ensures that channel consumption is respected regardless of context creation order or binding order
		// while ensuring that context layering still works
		for (auto& actionMetadata : m_ActionMetadata)
		{
			std::erase_if(actionMetadata.Triggers, [&](const ActionMetadata::TriggerMetadata& trigger)
			{
				for (auto [otherActionID, consumedChannel] : consumedChannels)
				{
					if (otherActionID != actionMetadata.Action.GetID() && trigger.Channel == consumedChannel)
					{
						// Remove this trigger if it uses a channel that has been marked as consumed
						// by a *different* action
						return true;
					}
				}

				return false;
			});
		}
	}

}
