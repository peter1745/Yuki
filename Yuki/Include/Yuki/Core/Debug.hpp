#pragma once

#include "Logging.hpp"

#include <source_location>

#if defined(YUKI_PLATFORM_WINDOWS)
	#define YUKI_DEBUG_BREAK __debugbreak()
#elif defined(YUKI_PLATFORM_LINUX)
	#include <signal.h>
	#define YUKI_DEBUG_BREAK raise(SIGTRAP)
#endif

#define YUKI_VERIFY(cond, ...) \
	if (!(cond))                                   \
	{                                              \
		std::source_location source_location = std::source_location::current(); \
		Yuki::LogError("Verify failed: {} at {}:{}", #cond, source_location.file_name(), source_location.line()); \
		YUKI_DEBUG_BREAK;                         \
	}
	