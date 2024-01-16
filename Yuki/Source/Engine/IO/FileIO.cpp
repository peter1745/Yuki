#include "FileIO.hpp"

namespace Yuki::FileIO {

	bool ReadText(const std::filesystem::path& filepath, std::string& outString)
	{
		std::ifstream stream(filepath);

		if (!stream)
		{
			return false;
		}

		std::stringstream str;
		str << stream.rdbuf();
		outString = str.str();
		return true;
	}

}
