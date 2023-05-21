#pragma once

#include "../Core/ResourceHandle.hpp"

namespace Yuki {

	enum class ShaderModuleType
	{
		None = -1,
		Vertex,
		Fragment
	};

	struct Shader
	{
		std::string Name;
		Map<ShaderModuleType, void*> ModuleHandles;
	};

}
