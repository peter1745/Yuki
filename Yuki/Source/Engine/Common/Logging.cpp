#include "Logging.hpp"

#include <spdlog/spdlog.h>

namespace Yuki {

	void Logging::Initialize()
	{
		spdlog::set_pattern("%^[%T][Yuki]: %v%$");
	}

	void Logging::LogInternal(LogLevel level, std::string_view message)
	{
		switch (level)
		{
		case Yuki::LogLevel::Info:
			spdlog::info(message);
			break;
		case Yuki::LogLevel::Warn:
			spdlog::warn(message);
			break;
		case Yuki::LogLevel::Error:
			spdlog::error(message);
			break;
		case Yuki::LogLevel::Fatal:
			spdlog::critical(message);
			break;
		default:
			break;
		}
	}

}
