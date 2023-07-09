#pragma once

#include "Yuki/Rendering/Mesh.hpp"

namespace Yuki {

	class MeshConverter
	{
	public:
		std::pair<std::filesystem::path, MeshScene> Convert(const std::filesystem::path& InFilePath) const;
	};

}
