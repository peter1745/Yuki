#pragma once

#include "Event.hpp"
#include "Yuki/IO/KeyCodes.hpp"

namespace Yuki {

	class GenericWindow;

	struct WindowResizeEvent : public Event
	{
		GenericWindow* Window;
		
		uint32_t OldWidth;
		uint32_t OldHeight;
		uint32_t NewWidth;
		uint32_t NewHeight;

		YUKI_EVENT_IMPL(WindowResize)
	};

	struct WindowCloseEvent : public Event
	{
		GenericWindow* Window;

		YUKI_EVENT_IMPL(WindowClose)
	};

	struct WindowMouseMoveEvent : public Event
	{
		GenericWindow* Window;
		int32_t MouseX;
		int32_t MouseY;

		YUKI_EVENT_IMPL(WindowMouseMove)
	};

	struct WindowMouseClickEvent : public Event
	{
		GenericWindow* Window;
		MouseButton Button;
		MouseButtonState State;
		int32_t MouseX;
		int32_t MouseY;

		YUKI_EVENT_IMPL(WindowMouseClick)
	};

	struct WindowKeyboardEvent : public Event
	{
		GenericWindow* Window;
		KeyCode Key;
		KeyState State;

		YUKI_EVENT_IMPL(WindowKeyboardInput)
	};

}
