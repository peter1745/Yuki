#include "StringHelper.hpp"

namespace Yuki {

	std::wstring StringHelper::WideFromUTF8(std::string_view string)
	{
#if defined(YUKI_PLATFORM_WINDOWS)
		int length = MultiByteToWideChar(CP_UTF8, 0, string.data(), static_cast<int>(string.length()), nullptr, 0);
		auto result = std::wstring(length, wchar_t(0));
		MultiByteToWideChar(CP_UTF8, 0, string.data(), static_cast<int>(string.length()), result.data(), length);
#else
		auto result = std::wstring(string.data(), string.length());
#endif

		return result;
	}

	std::string_view StringHelper::TrimWhitespace(std::string_view string)
	{
		constexpr std::string_view c_WhitespaceChars = " \n\r\t\f\v";

		size_t start = string.find_first_not_of(c_WhitespaceChars);
		string.remove_prefix(start);

		size_t End = string.find_last_not_of(c_WhitespaceChars);
		string.remove_suffix(string.length() - End - 1);

		return string;
	}

}
