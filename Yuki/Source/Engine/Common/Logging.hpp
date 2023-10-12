#pragma once

#include <format>
#include <stacktrace>

namespace Yuki {

	enum class LogLevel
	{
		Info,
		Warn,
		Error,
		Fatal
	};

	class Logging
	{
	public:
		template<typename... TArgs>
		static void Info(const std::format_string<TArgs...> format, TArgs&&... args)
		{
			auto message = std::format(format, std::forward<TArgs>(args)...);
			LogInternal(LogLevel::Info, message);
		}

		template<typename... TArgs>
		static void Warn(const std::format_string<TArgs...> format, TArgs&&... args)
		{
			auto message = std::format(format, std::forward<TArgs>(args)...);
			LogInternal(LogLevel::Warn, message);
		}

		template<typename... TArgs>
		static void Error(const std::format_string<TArgs...> format, TArgs&&... args)
		{
			auto message = std::format(format, std::forward<TArgs>(args)...);
			LogInternal(LogLevel::Error, message);
		}

		template<typename... TArgs>
		static void Fatal(const std::format_string<TArgs...> format, TArgs&&... args)
		{
			auto message = std::format(format, std::forward<TArgs>(args)...);
			LogInternal(LogLevel::Fatal, message);
		}

	private:
		static void Initialize();

		static void LogInternal(LogLevel level, std::string_view message);

	private:
		template<typename>
		friend struct AppRunner;
	};

#if defined(YUKI_PLATFORM_WINDOWS)
	#define YUKI_DEBUG_BREAK __debugbreak
#endif

#if defined(YUKI_DEBUG_BREAK)
	#define YUKI_VERIFY(Expr, ...) do {																				\
			if (!(Expr))																							\
			{																										\
				Logging::Error("Verify Failed ({}) at:\n{}", #Expr, std::to_string(std::stacktrace::current()));	\
				YUKI_DEBUG_BREAK();																					\
			}																										\
		} while(false)
#endif
}
