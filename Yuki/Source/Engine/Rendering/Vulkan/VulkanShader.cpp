#include "VulkanShader.hpp"
#include "VulkanRenderContext.hpp"

#include "Core/StringHelper.hpp"
#include "IO/FileIO.hpp"

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

namespace Yuki {

	static constexpr bool s_IncludeDebugInfo = true;

	static constexpr ShaderModuleType ShaderModuleTypeFromString(std::string_view InString)
	{
		if (InString == "vertex") return ShaderModuleType::Vertex;
		if (InString == "fragment") return ShaderModuleType::Fragment;

		return ShaderModuleType::None;
	}

	static constexpr EShLanguage ShaderModuleTypeToGLSLangType(ShaderModuleType InType)
	{
		switch (InType)
		{
		case ShaderModuleType::Vertex: return EShLangVertex;
		case ShaderModuleType::Fragment: return EShLangFragment;
		}

		return static_cast<EShLanguage>(-1);
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

	class GlslIncluder : public glslang::TShader::Includer
	{
	public:
		IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override
		{
			return nullptr;
		}

		IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override
		{
			return nullptr;
		}

		void releaseInclude(IncludeResult* includeResult) override
		{
			YUKI_UNUSED(includeResult);
		}
	};

	std::vector<uint32_t> CompileModule(ShaderModuleType InType, const std::filesystem::path& InFilePath, std::string_view InSource)
	{
		auto stage = ShaderModuleTypeToGLSLangType(InType);

		glslang::TShader shader{stage};
		shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
		shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
		shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_6);

		const char* source = InSource.data();
		int32_t sourceLength = InSource.length();
		auto filepath = InFilePath.string();
		const char* filePathStr = filepath.c_str();
		shader.setStringsWithLengthsAndNames(&source, &sourceLength, &filePathStr, 1);

		GlslIncluder includer;

		const auto* resource = GetDefaultResources();

		std::string preprocessed;
		if (!shader.preprocess(resource, 450, EEsProfile, false, false, EShMessages::EShMsgEnhanced, &preprocessed, includer))
		{
			LogError("Failed to preprocess shader: {}!", InFilePath.string());
			LogError("Reason: {}\n{}", shader.getInfoLog(), shader.getInfoDebugLog());
			YUKI_VERIFY(false);
		}

		const char* preprocessedStr = preprocessed.c_str();
		shader.setStrings(&preprocessedStr, 1);

		if (!shader.parse(resource, 450, ENoProfile, EShMessages::EShMsgDefault))
		{
			LogError("Failed to parse shader: {}!", InFilePath.string());
			LogError("Reason: {}\n{}", shader.getInfoLog(), shader.getInfoDebugLog());
			YUKI_VERIFY(false);
		}

		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(EShMessages(int(EShMessages::EShMsgSpvRules) | int(EShMessages::EShMsgVulkanRules))))
		{
			LogError("Failed to link shader: {}!", InFilePath.string());
			LogError("Reason: {}\n{}", program.getInfoLog(), program.getInfoDebugLog());
			YUKI_VERIFY(false);
		}

		glslang::SpvOptions spvOptions =
		{
			.generateDebugInfo = s_IncludeDebugInfo,
			.stripDebugInfo = !s_IncludeDebugInfo,
			.disableOptimizer = s_IncludeDebugInfo,
			.disassemble = false,
			.validate = true,
			.emitNonSemanticShaderDebugInfo = false,
			.emitNonSemanticShaderDebugSource = false,
		};

		const glslang::TIntermediate* intermediate = program.getIntermediate(stage);

		std::vector<uint32_t> bytecode;
		spv::SpvBuildLogger logger;
		glslang::GlslangToSpv(*intermediate, bytecode, &spvOptions);

		if (!logger.getAllMessages().empty())
		{
			LogWarn("SPIR-V messages generated for shader {}!", InFilePath.string());
			LogWarn("Message: {}", logger.getAllMessages());
		}

		return bytecode;

		/*shaderc::Compiler compiler;
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

		return result;*/
	}

	ShaderHandle VulkanRenderContext::CreateShader(const std::filesystem::path& InFilePath)
	{
		auto[handle, shader] = m_Shaders.Acquire();

		LogInfo("Compiling shader {}", InFilePath.string());

		std::string source = "";
		if (!FileIO::ReadText(InFilePath, source))
		{
			LogError("Couldn't compile shader, file: {} doesn't exist or couldn't be read!", InFilePath.string());
			return {};
		}

		auto shaderModules = ParseShaderSource(source);

		Map<ShaderModuleType, std::vector<uint32_t>> compiledByteCode;
		for (const auto& [shaderModuleType, moduleSource] : shaderModules)
		{
			auto result = CompileModule(shaderModuleType, InFilePath, moduleSource);

			/*if (result.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				compiledByteCode.clear();
				break;
			}*/

			compiledByteCode[shaderModuleType] = result;
		}

		shader.Name = InFilePath.stem().string();

		for (const auto& [shaderModuleType, moduleByteCode] : compiledByteCode)
		{
			VkShaderModuleCreateInfo moduleCreateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = moduleByteCode.size() * sizeof(uint32_t),
				.pCode = moduleByteCode.data()
			};

			vkCreateShaderModule(m_LogicalDevice, &moduleCreateInfo, nullptr, &shader.Modules[shaderModuleType]);
		}

		return handle;
	}

	ShaderHandle VulkanRenderContext::CreateShader(std::string_view InSource)
	{
		auto[handle, shader] = m_Shaders.Acquire();

		auto shaderModules = ParseShaderSource(InSource);

		Map<ShaderModuleType, std::vector<uint32_t>> compiledByteCode;
		for (const auto& [shaderModuleType, moduleSource] : shaderModules)
		{
			auto result = CompileModule(shaderModuleType, "", moduleSource);

			/*if (result.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				compiledByteCode.clear();
				break;
			}*/

			compiledByteCode[shaderModuleType] = result;
		}

		shader.Name = "Unnamed";

		for (const auto& [shaderModuleType, moduleByteCode] : compiledByteCode)
		{
			VkShaderModuleCreateInfo moduleCreateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = moduleByteCode.size() * sizeof(uint32_t),
				.pCode = moduleByteCode.data()
			};

			vkCreateShaderModule(m_LogicalDevice, &moduleCreateInfo, nullptr, &shader.Modules[shaderModuleType]);
		}

		return handle;
	}

	void VulkanRenderContext::Destroy(ShaderHandle InShader)
	{
		auto& shader = m_Shaders.Get(InShader);
		for (auto[moduleType, moduleHandle] : shader.Modules)
			vkDestroyShaderModule(m_LogicalDevice, moduleHandle, nullptr);
		m_Shaders.Return(InShader);
	}

}
