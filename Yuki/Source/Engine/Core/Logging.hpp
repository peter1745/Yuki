#pragma once

#include <format>

namespace Yuki {

	enum class LogLevel
	{
		Trace,
		Debug,
		Info,
		Warn,
		Error,
	};

	namespace Detail {
		void InitializeLogging();
		void LogMessage(std::string_view message, LogLevel level);
		void FlushMessages();
	}

	template<typename... Args>
	void WriteLine(const std::format_string<Args...> fmt, Args&&... args)
	{
		auto msg = std::format(fmt, std::forward<Args>(args)...);
		Detail::LogMessage(msg, LogLevel::Info);
	}

	template<typename... Args>
	void WriteLine(const std::format_string<Args...> fmt, LogLevel level, Args&&... args)
	{
		auto msg = std::format(fmt, std::forward<Args>(args)...);
		Detail::LogMessage(msg, level);
	}
}
