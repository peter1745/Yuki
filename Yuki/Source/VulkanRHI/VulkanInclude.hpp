#pragma once

#if defined(YUKI_PLATFORM_WINDOWS)
	#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(YUKI_PLATFORM_LINUX)
	#define VK_USE_PLATFORM_XCB_KHR
#endif

#include <volk/volk.h>

