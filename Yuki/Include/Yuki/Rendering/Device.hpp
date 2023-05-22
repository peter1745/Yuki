#pragma once

namespace Yuki {

	class Device
	{
	public:
		virtual ~Device() = default;

		virtual void WaitIdle() const = 0;

	};

}
