#include "WindowsUtils.hpp"

namespace Yuki {

	std::wstring WindowsUtils::ConvertUtf8ToWide(std::string_view InSource)
	{
		int length = MultiByteToWideChar(CP_UTF8, 0, InSource.data(), int32_t(InSource.length()), nullptr, 0);
		auto result = std::wstring(length, wchar_t(0));
		MultiByteToWideChar(CP_UTF8, 0, InSource.data(), int32_t(InSource.length()), result.data(), length);
		return result;
	}

}
