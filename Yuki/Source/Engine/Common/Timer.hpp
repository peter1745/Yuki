#pragma once

#include "Logging.hpp"

namespace Yuki::Timer {

	inline static thread_local std::chrono::steady_clock::time_point s_Start;
	inline static thread_local std::stacktrace s_Stacktrace;

	inline static void Start(std::stacktrace stacktrace = std::stacktrace::current())
	{
		s_Start = std::chrono::steady_clock::now();
		s_Stacktrace = stacktrace;
	}

	inline static void Stop(std::string_view name)
	{
		auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::steady_clock::now() - s_Start).count();
		Logging::Info("{}: {:.2f}s ({}ms) - {}", name, duration * 0.001, duration, std::to_string(s_Stacktrace[0]));
	}

}
