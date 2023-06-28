#pragma once

namespace Yuki {

	enum class EventType
	{
		ApplicationClose,
		
		WindowResize, WindowClose,
		WindowMouseMove, WindowMouseClick, WindowKeyboardInput,

		COUNT
	};

	class Event
	{
	public:
		virtual ~Event() = default;
		virtual EventType GetType() const = 0;
	};

}

#define YUKI_EVENT_IMPL(Type)               \
	EventType GetType() const override      \
	{                                       \
		return StaticType();                \
	}                                       \
	constexpr static EventType StaticType() \
	{                                       \
		return EventType::Type;             \
	}
