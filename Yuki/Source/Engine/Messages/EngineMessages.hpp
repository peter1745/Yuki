#pragma once

#include "Engine/Common/Core.hpp"

#include "MessageBase.hpp"

#include <concepts>

namespace Yuki {

	template<typename TMessageClass>
	concept MessageClass = std::derived_from<TMessageClass, MessageBase>;

	class EngineMessages
	{
		YUKI_MUTABLE_SINGLETON(EngineMessages);

	public:
		template<MessageClass TMessageClass>
		void Post(TMessageClass message)
		{
			size_t Index = MessageTraits::Index<TMessageClass>();
			m_MessageQueue[Index].emplace_back(std::move(Unique<TMessageClass>::New(message)));
		}

		template<typename TMessageClass>
		void AddListener(Function<void(const TMessageClass&)> func)
		{
			auto listener = Unique<MessageListenerLambdaFunc<TMessageClass>>::New(std::move(func));
			m_Listeners[listener->GetMessageType()].emplace_back(std::move(listener));
		}

		template<typename TListenerClass, typename TMessageClass>
		void AddListener(TListenerClass* listenerInstance, void(TListenerClass::*listenerFunc)(const TMessageClass&))
		{
			auto listener = Unique<MessageListenerMemberFunc<TListenerClass, TMessageClass>>::New(listenerInstance, listenerFunc);
			m_Listeners[listener->GetMessageType()].emplace_back(std::move(listener));
		}

		void ProcessMessages()
		{
			for (const auto& [messageType, messages] : m_MessageQueue)
			{
				for (const auto& message : messages)
				{
					for (auto& listener : m_Listeners[messageType])
						listener->Call(message.Get());
				}
			}
		}

	private:
		class MessageListenerFunc
		{
		public:
			virtual ~MessageListenerFunc() = default;

			virtual void Call(MessageBase* message) = 0;
			virtual size_t GetMessageType() const = 0;
		};

		template<typename TListenerClass, typename TMessageClass>
		class MessageListenerMemberFunc : public MessageListenerFunc
		{
		public:
			using ListenerFunc = void(TListenerClass::*)(const TMessageClass&);

		public:
			MessageListenerMemberFunc(TListenerClass* listenerInstance, ListenerFunc listenerFunc)
				: m_ListenerInstance(listenerInstance), m_ListenerFunc(listenerFunc)
			{
			}

			void Call(MessageBase* message) override
			{
				(m_ListenerInstance->*m_ListenerFunc)(*static_cast<TMessageClass*>(message));
			}

			size_t GetMessageType() const override { return MessageTraits::Index<TMessageClass>(); }

		private:
			TListenerClass* m_ListenerInstance;
			ListenerFunc m_ListenerFunc;
		};

		template<typename TMessageClass>
		class MessageListenerLambdaFunc : public MessageListenerFunc
		{
		public:
			using ListenerFunc = Function<void(const TMessageClass&)>;

		public:
			MessageListenerLambdaFunc(ListenerFunc listenerFunc)
				: m_ListenerFunc(std::move(listenerFunc))
			{
			}

			void Call(MessageBase* message) override
			{
				m_ListenerFunc(*static_cast<TMessageClass*>(message));
			}

			size_t GetMessageType() const override { return MessageTraits::Index<TMessageClass>(); }

		private:
			ListenerFunc m_ListenerFunc;
		};

	private:
		Map<size_t, DynamicArray<Unique<MessageBase>>> m_MessageQueue;
		Map<size_t, DynamicArray<Unique<MessageListenerFunc>>> m_Listeners;

		friend class Application;
	};

}
