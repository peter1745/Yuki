#pragma once

#include "Event.hpp"

namespace Yuki {

	struct WindowResizeEvent : public Event
	{
		uint32_t OldWidth;
		uint32_t OldHeight;
		uint32_t NewWidth;
		uint32_t NewHeight;

		YUKI_EVENT_IMPL(WindowResize)
	};

	struct WindowCloseEvent : public Event
	{
		YUKI_EVENT_IMPL(WindowClose)
	};

}
