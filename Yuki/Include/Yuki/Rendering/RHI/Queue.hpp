#pragma once

#include "Core/InitializerList.hpp"

#include "CommandBuffer.hpp"

#include <span>

namespace Yuki {

	class Viewport;
	class Fence;

	class Queue
	{
	public:
		virtual ~Queue() = default;

		virtual void SubmitCommandBuffers(const InitializerList<CommandBuffer>& InCommandBuffers, const InitializerList<Fence*> InWaits, const InitializerList<Fence*> InSignals) = 0;

		virtual void AcquireImages(std::span<Viewport* const> InViewports, const InitializerList<Fence*> InFences) = 0;
		virtual void Present(std::span<Viewport* const> InViewports, const InitializerList<Fence*> InFences) = 0;

		virtual void WaitIdle() const = 0;
	};

}
