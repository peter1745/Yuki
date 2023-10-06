#pragma once

#include <filesystem>

namespace Yuki::FileIO {

	bool ReadText(const std::filesystem::path& InFilePath, std::string& OutText);

}
