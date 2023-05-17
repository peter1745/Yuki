#pragma once

#include "Event.hpp"

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

}
