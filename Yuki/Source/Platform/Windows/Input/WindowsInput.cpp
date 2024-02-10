#include "Engine/Input/InputSystemImpl.hpp"

#include "GameInputProvider.hpp"

#include "Platform/Windows/WindowsCommon.hpp"

#include <chrono>

namespace Yuki {

	void RegisterPlatformInputProviders(InputSystem inputSystem)
	{
		inputSystem->RegisterProvider<GameInputProvider>();
	}

	rtmcpp::Vec2 InputSystem::GetCursorPosition() const
	{
		POINT position;
		YukiAssert(GetCursorPos(&position));
		return { static_cast<float32_t>(position.x), static_cast<float32_t>(position.y) };
	}

}
