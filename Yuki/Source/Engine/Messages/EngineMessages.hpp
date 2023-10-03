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
		void Post(TMessageClass InMessage)
		{
			auto Message = Unique<TMessageClass>::New(InMessage);
			size_t Index = MessageTraits::Index<TMessageClass>();
			m_MessageQueue[Index].emplace_back(std::move(Message));
		}

		template<typename TMessageClass>
		void AddListener(Function<void(const TMessageClass&)> InFunc)
		{
			auto Listener = Unique<MessageListenerLambdaFunc<TMessageClass>>::New(std::move(InFunc));
			m_Listeners[Listener->GetMessageType()].emplace_back(std::move(Listener));
		}

		template<typename TListenerClass, typename TMessageClass>
		void AddListener(TListenerClass* InListenerInstance, void(TListenerClass::*ListenerFunc)(const TMessageClass&))
		{
			auto Listener = Unique<MessageListenerMemberFunc<TListenerClass, TMessageClass>>::New(InListenerInstance, ListenerFunc);
			m_Listeners[Listener->GetMessageType()].emplace_back(std::move(Listener));
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

			virtual void Call(MessageBase* InMessage) = 0;
			virtual size_t GetMessageType() const = 0;
		};

		template<typename TListenerClass, typename TMessageClass>
		class MessageListenerMemberFunc : public MessageListenerFunc
		{
		public:
			using ListenerFunc = void(TListenerClass::*)(const TMessageClass&);

		public:
			MessageListenerMemberFunc(TListenerClass* InListenerInstance, ListenerFunc InListenerFunc)
				: m_ListenerInstance(InListenerInstance), m_ListenerFunc(InListenerFunc)
			{
			}

			void Call(MessageBase* InMessage) override
			{
				(m_ListenerInstance->*m_ListenerFunc)(*static_cast<TMessageClass*>(InMessage));
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
			MessageListenerLambdaFunc(ListenerFunc InListenerFunc)
				: m_ListenerFunc(std::move(InListenerFunc))
			{
			}

			void Call(MessageBase* InMessage) override
			{
				m_ListenerFunc(*static_cast<TMessageClass*>(InMessage));
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
