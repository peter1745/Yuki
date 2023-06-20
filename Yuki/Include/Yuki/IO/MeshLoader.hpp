#pragma once

#include "Yuki/Rendering/MeshData.hpp"

namespace Yuki {

	class MeshLoader
	{
	public:
		static MeshData LoadGLTFMesh(const std::filesystem::path& InFilePath);
	};

}
