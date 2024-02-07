#include "Engine/Input/InputSystemImpl.hpp"

#include "GameInputProvider.hpp"

namespace Yuki {

	void RegisterPlatformInputProviders(InputSystem inputSystem)
	{
		inputSystem->RegisterProvider<GameInputProvider>();
	}

}
