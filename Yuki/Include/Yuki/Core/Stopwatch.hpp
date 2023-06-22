#pragma once

#include "Logging.hpp"

#include <chrono>

thread_local inline std::chrono::steady_clock::time_point s_LastTime;
thread_local inline const char* s_StopwatchName;

#define YUKI_STOPWATCH_START() do { using namespace std::chrono; s_LastTime = steady_clock::now(); s_StopwatchName = __FUNCTION__; } while(0)
#define YUKI_STOPWATCH_START_N(name) do { using namespace std::chrono; s_LastTime = steady_clock::now(); s_StopwatchName = name; } while(0)
#define YUKI_STOPWATCH_STOP() do {\
									using namespace std::chrono;\
									auto elapsed = duration_cast<milliseconds>(steady_clock::now() - s_LastTime).count();\
									::Yuki::LogInfo("[{}]: {}ms", s_StopwatchName, elapsed);\
							  } while(0)
