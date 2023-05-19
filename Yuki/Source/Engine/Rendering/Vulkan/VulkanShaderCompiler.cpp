#include "VulkanShaderCompiler.hpp"

#include "IO/FileIO.hpp"

#include <shaderc/shaderc.hpp>

namespace Yuki {

	ResourceHandle<Shader> VulkanShaderCompiler::CompileFromFile(const std::filesystem::path& InFilePath)
	{
		//LogInfo("Compiling shader {}", InFilePath);

		std::string source = "";
		//if (!FileIO::ReadText(InFilePath, source))

		return ResourceHandle<Shader>::Invalid;
	}

}
