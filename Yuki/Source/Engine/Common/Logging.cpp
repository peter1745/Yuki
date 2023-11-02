#include "Logging.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Yuki {

	void Logging::Initialize()
	{
		auto& sinks = spdlog::default_logger()->sinks();
		sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Yuki.log"));

		spdlog::set_pattern("%^[%T][Yuki]: %v%$");
	}

	void Logging::LogInternal(LogLevel level, std::string_view message)
	{
		switch (level)
		{
		case Yuki::LogLevel::Info:
			spdlog::info(message);
			spdlog::default_logger()->flush();
			break;
		case Yuki::LogLevel::Warn:
			spdlog::warn(message);
			spdlog::default_logger()->flush();
			break;
		case Yuki::LogLevel::Error:
			spdlog::error(message);
			spdlog::default_logger()->flush();
			break;
		case Yuki::LogLevel::Fatal:
			spdlog::critical(message);
			spdlog::default_logger()->flush();
			break;
		default:
			break;
		}
	}

}
