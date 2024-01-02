#pragma once

#include "InputSystem.hpp"
#include "InputAdapter.hpp"

#include "Engine/Containers/Span.hpp"

namespace Yuki {

	struct InputMetadataBuilder : Handle<InputMetadataBuilder> {};

	struct ActionMetadata
	{
		InputAction Action;
		InputContext Context;

		InputReading Reading;

		struct TriggerMetadata
		{
			uint32_t AxisIndex;
			const ExternalInputChannel* Channel = nullptr;
			float32_t Scale;
		};

		std::vector<TriggerMetadata> Triggers;
	};

	template<>
	struct Handle<InputMetadataBuilder>::Impl
	{
		InputAdapter Adapter;
		std::vector<InputContext> Contexts;
		std::vector<ActionMetadata> Metadata;
		bool IsDirty = false;

		void BuildMetadata();
	};

	template<>
	struct Handle<InputSystem>::Impl
	{
		InputAdapter Adapter;
		InputMetadataBuilder MetadataBuilder;

		std::vector<InputAction> Actions;

		void Init();
		void Shutdown();

		void Update();
	};

}
