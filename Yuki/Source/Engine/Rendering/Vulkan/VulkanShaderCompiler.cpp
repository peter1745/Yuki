#include "VulkanShaderCompiler.hpp"

#include "Core/StringHelper.hpp"
#include "IO/FileIO.hpp"

#include <shaderc/shaderc.hpp>

namespace Yuki {

	static constexpr bool s_IncludeDebugInfo = true;

	static constexpr ShaderModuleType ShaderModuleTypeFromString(std::string_view InString)
	{
		if (InString == "vertex") return ShaderModuleType::Vertex;
		if (InString == "fragment") return ShaderModuleType::Fragment;

		return ShaderModuleType::None;
	}

	static constexpr shaderc_shader_kind ShaderModuleTypeToShaderCKind(ShaderModuleType InType)
	{
		switch (InType)
		{
		case ShaderModuleType::Vertex: return shaderc_vertex_shader;
		case ShaderModuleType::Fragment: return shaderc_fragment_shader;
		default:
			break;
		}

		return static_cast<shaderc_shader_kind>(-1);
	}

	Map<ShaderModuleType, std::string> ParseShaderSource(std::string_view InSource)
	{
		size_t stageStart = InSource.find("#stage");
		YUKI_VERIFY(stageStart != std::string_view::npos);

		Map<ShaderModuleType, std::string> result;

		while (stageStart != std::string_view::npos)
		{
			size_t stageNameStart = InSource.find_first_of(":", stageStart) + 1;
			size_t stageNameEnd = InSource.find_first_of("\n", stageNameStart);
			std::string_view stageName = InSource.substr(stageNameStart, stageNameEnd - stageNameStart);
			stageName = StringHelper::TrimWhitespace(stageName);

			ShaderModuleType shaderModuleType = ShaderModuleTypeFromString(stageName);
			YUKI_VERIFY(shaderModuleType != ShaderModuleType::None);
			YUKI_VERIFY(!result.contains(shaderModuleType));

			size_t stageEnd = InSource.find("#stage", stageNameEnd + 1);
			if (stageEnd == std::string_view::npos)
				stageEnd = InSource.length();

			result[shaderModuleType] = InSource.substr(stageNameEnd + 1, stageEnd - stageNameEnd - 1);

			stageStart = InSource.find("#stage", stageNameEnd + 1);
		}

		return result;
	}

	shaderc::SpvCompilationResult CompileModule(ShaderModuleType InType, std::string_view InSource)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions compilerOptions;
		compilerOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

		if constexpr (s_IncludeDebugInfo)
		{
			compilerOptions.SetGenerateDebugInfo();
			compilerOptions.SetOptimizationLevel(shaderc_optimization_level_zero);
		}
		else
		{
			compilerOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		compilerOptions.SetWarningsAsErrors();

		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(InSource.data(), ShaderModuleTypeToShaderCKind(InType), "", "main", compilerOptions);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LogError("Failed to compiler shader! Error: {}", result.GetErrorMessage());
		}

		return result;
	}

	VulkanShaderCompiler::VulkanShaderCompiler(ShaderManager* InShaderManager, VkDevice InDevice)
	    : m_ShaderManager(InShaderManager), m_Device(InDevice)
	{
	}

	ResourceHandle<Shader> VulkanShaderCompiler::CompileFromFile(const std::filesystem::path& InFilePath)
	{
		LogInfo("Compiling shader {}", InFilePath.string());

		std::string source = "";
		if (!FileIO::ReadText(InFilePath, source))
			return ResourceHandle<Shader>::Invalid;

		auto shaderModules = ParseShaderSource(source);

		Map<ShaderModuleType, std::vector<uint32_t>> compiledByteCode;
		for (const auto& [shaderModuleType, moduleSource] : shaderModules)
		{
			auto result = CompileModule(shaderModuleType, moduleSource);

			if (result.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				compiledByteCode.clear();
				break;
			}

			compiledByteCode[shaderModuleType] = std::vector<uint32_t>(result.begin(), result.end());
		}

		Shader* shader = new Shader();
		shader->Name = InFilePath.stem().string();

		for (const auto& [shaderModuleType, moduleByteCode] : compiledByteCode)
		{
			VkShaderModuleCreateInfo moduleCreateInfo = {
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = moduleByteCode.size() * sizeof(uint32_t),
				.pCode = moduleByteCode.data()
			};

			vkCreateShaderModule(m_Device, &moduleCreateInfo, nullptr, reinterpret_cast<VkShaderModule*>(&shader->ModuleHandles[shaderModuleType]));
		}

		return m_ShaderManager->AddShader(shader);
	}

}
