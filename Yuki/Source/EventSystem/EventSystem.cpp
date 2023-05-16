#include "EventSystem/EventSystem.hpp"

namespace Yuki {

	void EventSystem::PostEvent(Event* InEvent)
	{
		EventType eventType = InEvent->GetType();

		if (m_Listeners[(size_t)eventType].empty())
			return;

		for (const auto& listener : m_Listeners[(size_t)eventType])
			listener->Call(InEvent);
	}

}
