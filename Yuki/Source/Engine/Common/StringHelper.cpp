#include "StringHelper.hpp"

namespace Yuki {

	std::wstring StringHelper::WideFromUTF8(std::string_view InString)
	{
#if defined(YUKI_PLATFORM_WINDOWS)
		int Length = MultiByteToWideChar(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), nullptr, 0);
		auto Result = std::wstring(Length, wchar_t(0));
		MultiByteToWideChar(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), Result.data(), Length);
#else
		auto Result = std::wstring(InString.data(), InString.length());
#endif

		return Result;
	}

}
