#pragma once

#include "VulkanCommon.hpp"

#include <Engine/RHI/RHI.hpp>

#include <filesystem>

namespace Yuki {

	class ShaderCompiler
	{
	public:
		ShaderCompiler();
		~ShaderCompiler();

		VkShaderModule CompileShader(RHIContext context, const std::filesystem::path& filepath, ShaderStage stage);

	private:
	};

}
