#pragma once

#include "Yuki/Rendering/MeshData.hpp"

namespace Yuki {

	class RenderContext;

	class MeshLoader
	{
	public:
		static LoadedMesh LoadGLTFMesh(RenderContext* InContext, const std::filesystem::path& InFilePath);
	};

}
