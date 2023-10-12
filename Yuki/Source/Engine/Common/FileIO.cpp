#include "FileIO.hpp"

namespace Yuki::FileIO {

	bool ReadText(const std::filesystem::path& filepath, std::string& text)
	{
		std::ifstream stream(filepath);

		if (!stream)
			return false;

		std::stringstream contents;
		contents << stream.rdbuf();
		text = contents.str();
		return true;
	}

}
