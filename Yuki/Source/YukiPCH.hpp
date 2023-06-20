#include <iostream>
#include <fstream>
#include <sstream>

#include <string>
#include <filesystem>

#include <unordered_map>
#include <unordered_set>
#include <array>
#include <vector>

#include <functional>

#include <random>

#include <ranges>

#if defined(YUKI_PLATFORM_WINDOWS)
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

#include "Core/Core.hpp"
#include "Core/Debug.hpp"
#include "Core/Stopwatch.hpp"

#include "Memory/Unique.hpp"
