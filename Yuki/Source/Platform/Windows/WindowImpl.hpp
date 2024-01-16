#pragma once

#include "WindowsCommon.hpp"

#include "Engine/Core/Window.hpp"

namespace Yuki {

	template<>
	struct Handle<Window>::Impl
	{
		HWND WindowHandle;
		bool Closed = false;
	};

}
