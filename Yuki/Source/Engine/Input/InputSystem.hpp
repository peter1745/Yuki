#pragma once

#include "InputAction.hpp"
#include "InputContext.hpp"
#include "ExternalInputChannel.hpp"

namespace Yuki {

	struct InputSystem : Handle<InputSystem>
	{
		InputContext CreateContext();
		InputAction RegisterAction(const InputActionData& actionData);
	};

}
