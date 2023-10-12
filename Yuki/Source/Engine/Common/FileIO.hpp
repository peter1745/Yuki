#pragma once

#include <filesystem>

namespace Yuki::FileIO {

	bool ReadText(const std::filesystem::path& filepath, std::string& text);

}
