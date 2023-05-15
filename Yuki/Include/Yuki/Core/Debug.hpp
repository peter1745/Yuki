#pragma once

#include "Logging.hpp"

#include <source_location>

#if defined(YUKI_PLATFORM_WINDOWS)
	#define YUKI_DEBUG_BREAK __debugbreak()
#endif

#define YUKI_VERIFY(cond, ...) \
	if (!(cond))                                   \
	{                                              \
		std::source_location location = std::source_location::current(); \
		Yuki::LogError("Verify failed: {} at {}:{}", #cond, location.file_name(), location.line()); \
		YUKI_DEBUG_BREAK;                         \
	}