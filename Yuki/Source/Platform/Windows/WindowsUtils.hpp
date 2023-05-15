#pragma once

namespace Yuki {

	class WindowsUtils
	{
	public:
		static std::wstring ConvertUtf8ToWide(std::string_view InSource);
	};

}
