#pragma once

#include "KeyCodes.hpp"

namespace Yuki {

	enum class InputEventType { Invalid, Key };

	struct InputEvent
	{
		InputEventType Type;

		InputEvent(InputEventType InType)
			: Type(InType) {}
	};

	struct KeyInputEvent : public InputEvent
	{
		KeyCode Key;

		KeyInputEvent(KeyCode InKey)
			: InputEvent(InputEventType::Key), Key(InKey) {}
	};

	struct InputBinding
	{
		virtual bool TryHandle(const InputEvent* InEvent) const = 0;
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

		RangedInput(const Binding& InNegative, const Binding& InPositive, Function<void(float)> InHandler)
			: Negative(InNegative), Positive(InPositive), Handler(InHandler) {}

		bool TryHandle(const InputEvent* InEvent) const override
		{
			switch (InEvent->Type)
			{
			case InputEventType::Key:
			{
				const auto* KeyEvent = Cast<const KeyInputEvent*>(InEvent);
				if (KeyEvent->Key == Negative.Key)
				{
					Handler(Negative.Value);
					return true;
				}
				else if (KeyEvent->Key == Positive.Key)
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
