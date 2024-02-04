#pragma once

#include "Engine/RHI/RHI.hpp"

#include <filesystem>

namespace Yuki {

	class ImageProcessor
	{
	public:
		Image CreateFromFile(RHIContext context, const std::filesystem::path& filepath) const;
	};

}
