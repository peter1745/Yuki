#include "Core/StringHelper.hpp"

namespace Yuki {

	static constexpr std::string_view c_WhitespaceChars = " \n\r\t\f\v";

	std::string_view StringHelper::TrimWhitespace(std::string_view InString)
	{
		size_t start = InString.find_first_not_of(c_WhitespaceChars);
		InString.remove_prefix(start);

		size_t end = InString.find_last_not_of(c_WhitespaceChars);
		InString.remove_suffix(InString.length() - end - 1);

		return InString;
	}

}
