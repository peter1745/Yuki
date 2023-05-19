#pragma once

#include "Rendering/ShaderCompiler.hpp"

namespace Yuki {

	class VulkanShaderCompiler : public ShaderCompiler
	{
	public:
		ResourceHandle<Shader> CompileFromFile(const std::filesystem::path& InFilePath) override;
	};

}
