#pragma once

#include "../Core/ResourceHandle.hpp"

namespace Yuki {

	struct Shader
	{
	};

	class ShaderCompiler
	{
	public:
		virtual ResourceHandle<Shader> CompileFromFile(const std::filesystem::path& InFilePath) = 0;
	};

}
