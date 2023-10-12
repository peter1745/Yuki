#pragma once

#include "KeyCodes.hpp"

namespace Yuki {

	enum class InputEventType { Invalid, Key };

	struct InputEvent
	{
		InputEventType Type;

		InputEvent(InputEventType type)
			: Type(type) {}
	};

	struct KeyInputEvent : public InputEvent
	{
		KeyCode Key;

		KeyInputEvent(KeyCode key)
			: InputEvent(InputEventType::Key), Key(key) {}
	};

	struct InputBinding
	{
		virtual bool TryHandle(const InputEvent* event) const = 0;
	};

	struct RangedInput : public InputBinding
	{
		struct Binding
		{
			float Value;

			KeyCode Key = KeyCode::Unknown;
		};

		Binding Negative;
		Binding Positive;
		Function<void(float)> Handler;

		RangedInput(const Binding& negative, const Binding& positive, Function<void(float)> handler)
			: Negative(negative), Positive(positive), Handler(handler) {}

		bool TryHandle(const InputEvent* event) const override
		{
			switch (event->Type)
			{
			case InputEventType::Key:
			{
				const auto* keyEvent = Cast<const KeyInputEvent*>(event);
				if (keyEvent->Key == Negative.Key)
				{
					Handler(Negative.Value);
					return true;
				}
				else if (keyEvent->Key == Positive.Key)
				{
					Handler(Positive.Value);
					return true;
				}

				break;
			}
			}

			return false;
		}
	};

}
