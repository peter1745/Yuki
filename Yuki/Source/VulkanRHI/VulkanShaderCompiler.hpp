#pragma once

#include "VulkanInclude.hpp"

#include "Engine/RHI/RenderHandles.hpp"

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

		VkShaderModule CompileOrGetModule(VkDevice device, const std::filesystem::path& filepath, ShaderStage stage);

	private:
		HashMap<ShaderStage, std::string> PreProcessSource(std::string_view source) const;
		DynamicArray<uint32_t> CompileStage(const std::filesystem::path& filepath, ShaderStage stage, std::string_view source) const;
		void CompileModules(VkDevice device, const std::filesystem::path& filepath);

	private:
		HashMap<std::filesystem::path, VulkanShaderFile> m_CompiledFiles;
	};

}
