#include "Logging.hpp"

#include <spdlog/spdlog.h>

namespace Yuki {

	void Logging::Initialize()
	{
		spdlog::set_pattern("%^[%T][Yuki]: %v%$");
	}

	void Logging::LogInternal(LogLevel InLevel, std::string_view InMessage)
	{
		switch (InLevel)
		{
		case Yuki::LogLevel::Info:
			spdlog::info(InMessage);
			break;
		case Yuki::LogLevel::Warn:
			spdlog::warn(InMessage);
			break;
		case Yuki::LogLevel::Error:
			spdlog::error(InMessage);
			break;
		case Yuki::LogLevel::Fatal:
			spdlog::critical(InMessage);
			break;
		default:
			break;
		}
	}

}
