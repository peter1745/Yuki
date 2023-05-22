#pragma once

#include "Shader.hpp"

namespace Yuki {

	class ShaderCompiler
	{
	public:
		virtual ~ShaderCompiler() = default;

		virtual ResourceHandle<Shader> CompileFromFile(const std::filesystem::path& InFilePath) = 0;
	};

}
