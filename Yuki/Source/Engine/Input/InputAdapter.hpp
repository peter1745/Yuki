#pragma once

#include "InputDevice.hpp"

namespace Yuki {

	class InputAdapter
	{
	public:
		InputAdapter();
		~InputAdapter();

		void Update();

		uint32_t GetDeviceCount() const;
		const InputDevice* GetDevice(uint32_t deviceIndex) const;
	};

}
