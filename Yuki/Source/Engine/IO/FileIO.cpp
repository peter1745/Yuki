#include "IO/FileIO.hpp"

namespace Yuki {

	bool FileIO::ReadText(const std::filesystem::path& InFilePath, std::string& OutText)
	{
		std::ifstream stream(InFilePath);

		if (!stream)
			return false;

		std::stringstream stringStream;
		stringStream << stream.rdbuf();
		OutText = stringStream.str();

		stream.close();

		return true;
	}

}
