#pragma once

#include "Engine/RHI/PipelineInfo.hpp"

#include "VulkanInclude.hpp"

namespace Yuki::RHI {

	struct VulkanShaderFile
	{
		std::string Name;
		HashMap<ShaderStage, DynamicArray<uint32_t>> ByteCodes;
		HashMap<ShaderStage, VkShaderModule> Modules;
	};

	class VulkanShaderCompiler
	{
	public:
		VulkanShaderCompiler();
		~VulkanShaderCompiler();

		VkShaderModule CompileOrGetModule(VkDevice InDevice, const std::filesystem::path& InFilePath, ShaderStage InStage);

	private:
		HashMap<ShaderStage, std::string> PreProcessSource(std::string_view InSource) const;
		DynamicArray<uint32_t> CompileStage(const std::filesystem::path& InFilePath, ShaderStage InStage, std::string_view InSource) const;
		void CompileModules(VkDevice InDevice, const std::filesystem::path& InFilePath);

	private:
		HashMap<std::filesystem::path, VulkanShaderFile> m_CompiledFiles;
	};

}
