#pragma once

namespace Yuki {

	class FileIO
	{
	public:
		static bool ReadText(const std::filesystem::path& InFilePath, std::string& OutText);
	};

}
