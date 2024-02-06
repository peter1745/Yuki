#include "InputSystem.hpp"
#include "InputSystemImpl.hpp"

#include <ranges>

namespace Yuki {

	template<>
	struct Handle<InputContext>::Impl
	{
		bool IsActive = false;

		std::vector<InputAction> Actions;
		std::unordered_map<InputAction::ID, InputActionFunction> ActionFunctions;

		InputSystem System;

		void InvokeAction(InputAction action, const InputReading& reading)
		{
			ActionFunctions[action.GetID()](reading);
		}
	};

	template<>
	struct Handle<InputAction>::Impl
	{
		InputActionData Data;
		InputSystem System;
	};

	void InputContext::Activate()
	{
		m_Impl->IsActive = true;
		m_Impl->System->NotifyDataChange();
	}

	void InputContext::Deactivate()
	{
		m_Impl->IsActive = false;
		m_Impl->System->NotifyDataChange();
	}

	void InputContext::BindAction(InputAction action, InputActionFunction&& func)
	{
		m_Impl->Actions.push_back(action);
		m_Impl->ActionFunctions[action.GetID()] = std::move(func);
		m_Impl->System->NotifyDataChange();
	}

	void InputAction::AddTrigger(uint32_t axis, const TriggerBinding& triggerBinding)
	{
		if (axis >= m_Impl->Data.AxisBindings.size())
		{
			return;
		}

		m_Impl->Data.AxisBindings[axis].Bindings.push_back(triggerBinding);

		m_Impl->System->NotifyDataChange();
	}

	void InputAction::RemoveTrigger(uint32_t axis, uint32_t trigger)
	{
		if (axis >= m_Impl->Data.AxisBindings.size())
		{
			return;
		}

		auto& bindings = m_Impl->Data.AxisBindings[axis].Bindings;

		if (trigger >= bindings.size())
		{
			return;
		}

		bindings.erase(std::next(bindings.begin(), trigger));

		m_Impl->System->NotifyDataChange();
	}

	void InputAction::ReplaceTrigger(uint32_t axis, uint32_t trigger, TriggerID triggerID)
	{
		if (axis >= m_Impl->Data.AxisBindings.size())
		{
			return;
		}

		auto& bindings = m_Impl->Data.AxisBindings[axis].Bindings;

		if (trigger >= bindings.size())
		{
			return;
		}

		bindings[trigger].ID = triggerID;

		m_Impl->System->NotifyDataChange();
	}

	void InputSystem::Impl::Init()
	{
		Adapter = InputAdapter::Create();
	}

	void InputSystem::Impl::Shutdown()
	{
		Adapter.Destroy();
	}

	InputAction InputSystem::RegisterAction(const InputActionData& actionData)
	{
		auto* action = new InputAction::Impl();
		action->Data = actionData;
		action->System = { m_Impl };

		m_Impl->Actions.push_back({ action });
		return { action };
	}

	InputContext InputSystem::CreateContext()
	{
		auto* contextImpl = new InputContext::Impl();
		contextImpl->System = { m_Impl };

		m_Impl->Contexts.push_back({ contextImpl });
		return { contextImpl };
	}

	static bool ShouldTrigger(TriggerEventType eventType, const ExternalInputChannel* channel)
	{
		using enum TriggerEventType;

		if (eventType & OnPressed  && channel->Value != 0.0f && channel->PreviousValue == 0.0f) return true;
		if (eventType & OnHeld     && channel->Value != 0.0f && channel->PreviousValue != 0.0f) return true;
		if (eventType & OnReleased && channel->Value == 0.0f && channel->PreviousValue != 0.0f) return true;

		//if (eventType & OnCanceled) return true;
		return false;
	}

	void InputSystem::Impl::Update()
	{
		// TODO(Peter): Change this to allow for multiple backends
		Adapter.Update();

		if (NeedsRecompile)
		{
			CompileInputActions();
			NeedsRecompile = false;
		}

		// Dispatch input events to the active actions
		for (auto& action : CompiledActions)
		{
			bool triggered = false;

			for (const auto& trigger : action.Triggers)
			{
				// If the channel hasn't registered any input for this or the previous frame just continue
				if (!trigger.Channel->HasRegisteredInput())
					continue;

				if (!ShouldTrigger(trigger.EventType, trigger.Channel))
					continue;

				action.Reading.Write(trigger.AxisIndex, trigger.Channel->Value * trigger.Scale);
				triggered = true;
			}

			if (!triggered)
				continue;

			// Dispatch the reading to the bound action
			action.Context->InvokeAction(action.Action, action.Reading);
		}
	}

	void InputSystem::Impl::NotifyDataChange()
	{
		NeedsRecompile = true;
	}

	void InputSystem::Impl::CompileInputActions()
	{
		CompiledActions.clear();

		std::unordered_map<InputAction::ID, const ExternalInputChannel*> consumedChannels;

		for (auto context : Contexts)
		{
			if (!context->IsActive)
				continue;

			for (auto action : context->Actions)
			{
				auto& compiledAction = CompiledActions.emplace_back();
				compiledAction.Action = action;
				compiledAction.Context = context;
				compiledAction.Reading = InputReading(action->Data.AxisBindings.size());

				for (uint32_t axisIndex = 0; axisIndex < action->Data.AxisBindings.size(); axisIndex++)
				{
					const auto& axisBinding = action->Data.AxisBindings[axisIndex];

					for (const auto& triggerBinding : axisBinding.Bindings)
					{
						const auto device = Adapter.GetDevice(triggerBinding.ID.DeviceID);

						if (!device)
							continue;

						const auto* channel = device.GetChannel(triggerBinding.ID.InputID);

						if (channel == nullptr)
							continue;

						// If our channel has been marked as consumed already we just ignore this trigger (other triggers may still work)
						bool usesConsumedChannel = false;
						for (auto [otherActionID, consumedChannel] : consumedChannels)
						{
							if (otherActionID != action.GetID() && channel == consumedChannel)
							{
								usesConsumedChannel = true;
								break;
							}
						}

						if (usesConsumedChannel)
							continue;

						auto& actionTrigger = compiledAction.Triggers.emplace_back();
						actionTrigger.AxisIndex = axisIndex;
						actionTrigger.Channel = channel;
						actionTrigger.Scale = triggerBinding.Scale;
						actionTrigger.EventType = triggerBinding.ID.EventType;

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
		for (auto& compiledAction : CompiledActions)
		{
			std::erase_if(compiledAction.Triggers, [&](const CompiledAction::TriggerMetadata& trigger)
			{
				for (auto [otherActionID, consumedChannel] : consumedChannels)
				{
					if (otherActionID != compiledAction.Action.GetID() && trigger.Channel == consumedChannel)
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
