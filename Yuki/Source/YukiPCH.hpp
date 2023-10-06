#include <iostream>
#include <fstream>
#include <sstream>

#include <string>
#include <filesystem>

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <vector>

#include <functional>

#include <cmath>
#include <math.h>

#include <random>

#include <limits>

#include <ranges>
#include <chrono>
#include <algorithm>

#if defined(YUKI_PLATFORM_WINDOWS)
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <hidusage.h>
#endif

#include "Engine/Common/Core.hpp"
#include "Engine/Common/Logging.hpp"
