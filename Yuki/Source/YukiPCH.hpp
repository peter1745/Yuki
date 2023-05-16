#include <iostream>

#include <string>

#include <unordered_map>
#include <unordered_set>
#include <array>
#include <vector>

#include <functional>

#if defined(YUKI_PLATFORM_WINDOWS)
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

#include "Core/Debug.hpp"

#include "Containers/Array.hpp"
#include "Containers/List.hpp"

#include "Memory/Unique.hpp"
