#pragma once

#include "Engine/Core/Unique.hpp"

#include "InputAction.hpp"
#include "InputContext.hpp"
#include "ExternalInputChannel.hpp"

namespace Yuki {

	struct InputSystem : Handle<InputSystem>
	{
		InputAction RegisterAction(const InputActionData& actionData);
		InputContext CreateContext();
	};

}
