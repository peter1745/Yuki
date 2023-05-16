#pragma once

#include "Event.hpp"

namespace Yuki {

	struct ApplicationCloseEvent : public Event
	{
		YUKI_EVENT_IMPL(ApplicationClose)
	};

}
