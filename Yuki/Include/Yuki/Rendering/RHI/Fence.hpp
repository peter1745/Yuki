#pragma once

namespace Yuki {

	class Fence
	{
	public:
		virtual ~Fence() = default;

		virtual void Wait(uint64_t InValue = 0) = 0;
		virtual uint64_t& GetValue() = 0;
	};

}
