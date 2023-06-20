#pragma once

#include <Yuki/Math/Vec3.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

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

namespace fmt {

	template<>
	struct formatter<Yuki::Math::Vec3>
	{
		char presentation = 'f';

		constexpr auto parse(format_parse_context& InCTX) -> decltype(InCTX.begin())
		{
			auto it = InCTX.begin(), end = InCTX.end();

			if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;
			if (it != end && *it != '}') throw format_error("invalid format");

			return it;
		}

		template <typename FormatContext>
		auto format(const Yuki::Math::Vec3& InVec, FormatContext& InCTX) const -> decltype(InCTX.out())
		{
			return presentation == 'f'
				? fmt::format_to(InCTX.out(), "({:.3f}, {:.3f}, {:.3f})", InVec.X, InVec.Y, InVec.Z)
				: fmt::format_to(InCTX.out(), "({:.3e}, {:.3e}, {:.3e})", InVec.X, InVec.Y, InVec.Z);
		}
	};

}
