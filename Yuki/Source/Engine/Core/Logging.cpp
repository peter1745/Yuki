#include "Logging.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Yuki::Detail {

	void InitializeLogging()
	{
		auto& sinks = spdlog::default_logger()->sinks();
		sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Logs/Messages.log", true));

		spdlog::set_pattern("%^[%T][Yuki]: %v%$");

		using namespace std::chrono_literals;
		spdlog::flush_every(5s);
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

	void FlushMessages()
	{
		spdlog::default_logger()->flush();
	}

}
