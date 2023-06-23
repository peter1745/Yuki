#pragma once

#include "Event.hpp"

#include "../Core/Core.hpp"
#include "../Memory/Unique.hpp"

namespace Yuki {

	class EventListenerFunc
	{
	public:
		virtual ~EventListenerFunc() = default;

		virtual void Call(Event* InEvent) = 0;
	};

	template<typename TListenerType, typename TEventType>
	class EventListenerMemberFunc : public EventListenerFunc
	{
	public:
		using ListenerFunc = void(TListenerType::*)(const TEventType&);

	public:
		EventListenerMemberFunc(TListenerType* InListenerInstance, ListenerFunc InListenerFunc)
			: m_ListenerInstance(InListenerInstance), m_ListenerFunc(InListenerFunc)
		{
		}

		void Call(Event* InEvent)
		{
			(m_ListenerInstance->*m_ListenerFunc)(*((TEventType*)InEvent));
		}

	private:
		TListenerType* m_ListenerInstance;
		ListenerFunc m_ListenerFunc;
	};

	class EventSystem
	{
	public:
		template<typename TListenerType, typename TEventType>
		void AddListener(TListenerType* InListenerInstance, void(TListenerType::*ListenerFunc)(const TEventType&))
		{
			EventType eventType = TEventType::StaticType();
			auto eventListenerFunc = Unique<EventListenerMemberFunc<TListenerType, TEventType>>::Create(InListenerInstance, ListenerFunc);
			m_Listeners[(size_t)eventType].emplace_back(std::move(eventListenerFunc));
		}

		void PostEvent(Event* InEvent);

	private:
		Array<DynamicArray<Unique<EventListenerFunc>>, (size_t)EventType::COUNT> m_Listeners;
	};

}
