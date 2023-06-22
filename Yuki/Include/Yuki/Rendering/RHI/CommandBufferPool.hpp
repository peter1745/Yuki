#pragma once

#include "CommandBuffer.hpp"

namespace Yuki {

	struct CommandBufferPoolInfo
	{
		bool IsTransient = false;
	};

	class CommandBufferPool
	{
	public:
		virtual ~CommandBufferPool() = default;

		virtual CommandBuffer* NewCommandBuffer() = 0;

		virtual void Reset() = 0;
	};

}
