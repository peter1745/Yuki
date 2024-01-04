#include "Logging.hpp"

#include <spdlog/spdlog.h>

namespace Yuki::Detail {

	void InitializeLogging()
	{
		spdlog::set_pattern("%^[%T][Yuki]: %v%$");
	}

	void LogMessage(std::string_view message, LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Trace:
		{
			spdlog::trace(message);
			break;
		}
		case LogLevel::Debug:
		{
			spdlog::debug(message);
			break;
		}
		case LogLevel::Info:
		{
			spdlog::info(message);
			break;
		}
		case LogLevel::Warn:
		{
			spdlog::warn(message);
			break;
		}
		case LogLevel::Error:
		{
			spdlog::error(message);
			break;
		}
		}
	}

}
