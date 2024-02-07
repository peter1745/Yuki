#pragma once

#include "InputSystem.hpp"
#include "InputDevice.hpp"

#include <concepts>

namespace Yuki {

	struct CompiledAction
	{
		InputAction Action;
		InputContext Context;

		InputReading Reading;

		struct TriggerMetadata
		{
			uint32_t AxisIndex;
			const ExternalInputChannel* Channel = nullptr;
			float32_t Scale;
			TriggerEventType EventType;
		};

		std::vector<TriggerMetadata> Triggers;
	};

	template<>
	struct Handle<InputSystem>::Impl
	{
		InputDeviceRegistry DeviceRegistry;
		std::vector<Aura::Unique<InputProvider>> Providers;

		std::vector<InputContext> Contexts;
		std::vector<InputAction> Actions;

		bool NeedsRecompile = false;
		std::vector<CompiledAction> CompiledActions;

		void Init();
		void Shutdown();

		void Update();

		void NotifyDataChange();
		void CompileInputActions();

		template<typename Provider>
		void RegisterProvider()
		{
			auto* provider = new Provider();
			provider->Init(DeviceRegistry);
			Providers.emplace_back(provider);
		}
	};

}
