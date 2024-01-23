#pragma once

#include "InputSystem.hpp"
#include "InputAdapter.hpp"

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
		InputAdapter Adapter;
		std::vector<InputContext> Contexts;
		std::vector<InputAction> Actions;

		bool NeedsRecompile = false;
		std::vector<CompiledAction> CompiledActions;

		void Init();
		void Shutdown();

		void Update();

		void NotifyDataChange();
		void CompileInputActions();
	};

}
