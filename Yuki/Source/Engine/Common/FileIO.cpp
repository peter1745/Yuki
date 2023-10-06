#include "FileIO.hpp"

namespace Yuki::FileIO {

	bool ReadText(const std::filesystem::path& InFilePath, std::string& OutText)
	{
		std::ifstream Stream(InFilePath);

		if (!Stream)
			return false;

		std::stringstream Contents;
		Contents << Stream.rdbuf();
		OutText = Contents.str();
		return true;
	}

}
