#pragma once

#include <string>

namespace Yuki {

	class StringHelper
	{
	public:
		static std::wstring WideFromUTF8(std::string_view string);
		static std::string_view TrimWhitespace(std::string_view string);

	public:
		StringHelper() = delete;
		StringHelper(const StringHelper&) = delete;
		StringHelper(StringHelper&&) noexcept = delete;

		StringHelper& operator=(const StringHelper&) = delete;
		StringHelper& operator=(StringHelper&&) noexcept = delete;
	};

}
