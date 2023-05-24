#pragma once

#include <spdlog/spdlog.h>

namespace Yuki {

	inline void LogInit()
	{
		spdlog::set_pattern("%^[%T][Yuki]: %v%$");
	}

	template <typename... TArgs>
	constexpr void LogDebug(spdlog::format_string_t<TArgs...> InFormat, TArgs&&... InArgs)
	{
		spdlog::debug(InFormat, std::forward<TArgs>(InArgs)...);
	}

	template<typename... TArgs>
	constexpr void LogInfo(spdlog::format_string_t<TArgs...> InFormat, TArgs&&... InArgs)
	{
		spdlog::info(InFormat, std::forward<TArgs>(InArgs)...);
	}

	template <typename... TArgs>
	constexpr void LogWarn(spdlog::format_string_t<TArgs...> InFormat, TArgs&&... InArgs)
	{
		spdlog::warn(InFormat, std::forward<TArgs>(InArgs)...);
	}

	template <typename... TArgs>
	constexpr void LogError(spdlog::format_string_t<TArgs...> InFormat, TArgs&&... InArgs)
	{
		spdlog::error(InFormat, std::forward<TArgs>(InArgs)...);
	}

}
