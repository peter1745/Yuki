#include "VulkanShaderCompiler.hpp"
#include "Engine/Common/StringHelper.hpp"
#include "Engine/Common/FileIO.hpp"

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

namespace Yuki::RHI {

	static const HashMap<std::string_view, ShaderStage> c_StageLookup =
	{
		{ "Vertex",			ShaderStage::Vertex },
		{ "Fragment",		ShaderStage::Fragment },
		{ "RTRayGen",		ShaderStage::RayGeneration },
		{ "RTMiss",			ShaderStage::RayMiss },
		{ "RTClosestHit",	ShaderStage::RayClosestHit },
		{ "RTAnyHit",		ShaderStage::RayAnyHit },
	};

	static const HashMap<ShaderStage, EShLanguage> c_EShLanguageLookup =
	{
		{ ShaderStage::Vertex,			EShLangVertex },
		{ ShaderStage::Fragment,		EShLangFragment },
		{ ShaderStage::RayGeneration,	EShLangRayGen },
		{ ShaderStage::RayMiss,			EShLangMiss },
		{ ShaderStage::RayClosestHit,	EShLangClosestHit },
		{ ShaderStage::RayAnyHit,		EShLangAnyHit },
	};

	VulkanShaderCompiler::VulkanShaderCompiler()
	{
		glslang::InitializeProcess();
	}

	VulkanShaderCompiler::~VulkanShaderCompiler()
	{
		glslang::FinalizeProcess();
	}

	VkShaderModule VulkanShaderCompiler::CompileOrGetModule(VkDevice device, const std::filesystem::path& filepath, ShaderStage stage)
	{
		if (!m_CompiledFiles.contains(filepath))
			CompileModules(device, filepath);

		return m_CompiledFiles.at(filepath).Modules.at(stage);
	}

	HashMap<ShaderStage, std::string> VulkanShaderCompiler::PreProcessSource(std::string_view source) const
	{
		size_t stageStart = source.find("#stage");
		YUKI_VERIFY(stageStart != std::string_view::npos);

		HashMap<ShaderStage, std::string> result;

		while (stageStart != std::string_view::npos)
		{
			size_t stageNameStart = stageStart + strlen("#stage");
			size_t stageNameEnd = source.find_first_of("\n", stageNameStart);
			std::string_view stageName = source.substr(stageNameStart, stageNameEnd - stageNameStart);
			stageName = StringHelper::TrimWhitespace(stageName);

			YUKI_VERIFY(c_StageLookup.contains(stageName));

			ShaderStage stage = c_StageLookup.at(stageName);

			size_t stageEnd = source.find("#stage", stageNameEnd + 1);
			if (stageEnd == std::string_view::npos)
				stageEnd = source.length();

			result[stage] = source.substr(stageNameEnd + 1, stageEnd - stageNameEnd - 1);

			stageStart = source.find("#stage", stageNameEnd + 1);
		}

		return result;
	}

	class GlslIncluder : public glslang::TShader::Includer
	{
		struct UserData
		{
			std::string Name;
			std::string Content;
		};

	public:
		IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override
		{
			return include(headerName, includerName, false);
		}

		IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override
		{
			return include(headerName, includerName, true);
		}

		void releaseInclude(IncludeResult* includeResult) override
		{
			delete Cast<UserData*>(includeResult->userData);
			delete includeResult;
		}

	private:
		IncludeResult* include(const char* headerName, const char* includerName, bool isRelative)
		{
			std::filesystem::path requested = headerName;
			std::filesystem::path current = includerName;

			std::filesystem::path target;
			bool exists = false;

			if (isRelative)
			{
				target = current.parent_path() / requested;
				exists = std::filesystem::exists(target);
			}

			if (!exists)
			{
				target = requested;
				exists = std::filesystem::exists(target);
			}

			auto userData = new UserData();

			if (exists)
			{
				userData->Name = target.string();
				FileIO::ReadText(target, userData->Content);
			}
			else
			{
				YUKI_VERIFY(false);
			}

			return new IncludeResult(userData->Name, userData->Content.c_str(), userData->Content.length(), userData);
		}
	};

	DynamicArray<uint32_t> VulkanShaderCompiler::CompileStage(const std::filesystem::path& filepath, ShaderStage stage, std::string_view source) const
	{
		YUKI_VERIFY(c_EShLanguageLookup.contains(stage));

		auto lang = c_EShLanguageLookup.at(stage);

		glslang::TShader shader{ lang };
		shader.setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
		shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
		shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_6);

		const char* Source = source.data();
		int32_t SourceLength = Cast<int32_t>(source.length());
		auto FilePath = filepath.string();
		const char* FilePathStr = FilePath.c_str();
		shader.setStringsWithLengthsAndNames(&Source, &SourceLength, &FilePathStr, 1);

		GlslIncluder includer;

		const auto* resource = GetDefaultResources();

		std::string preProcessed;
		if (!shader.preprocess(resource, 450, EEsProfile, false, false, EShMessages::EShMsgEnhanced, &preProcessed, includer))
		{
			Logging::Error("Failed to preprocess shader: {}!", filepath.string());
			Logging::Error("Reason: {}\n{}", shader.getInfoLog(), shader.getInfoDebugLog());
			YUKI_VERIFY(false);
		}

		const char* preProcessedStr = preProcessed.c_str();
		shader.setStrings(&preProcessedStr, 1);

		if (!shader.parse(resource, 450, false, EShMessages::EShMsgDefault))
		{
			Logging::Error("Failed to parse shader: {}!", filepath.string());
			Logging::Error("Reason: {}\n{}", shader.getInfoLog(), shader.getInfoDebugLog());
			YUKI_VERIFY(false);
		}

		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(EShMessages(int(EShMessages::EShMsgSpvRules) | int(EShMessages::EShMsgVulkanRules))))
		{
			Logging::Error("Failed to link shader: {}!", filepath.string());
			Logging::Error("Reason: {}\n{}", shader.getInfoLog(), shader.getInfoDebugLog());
			YUKI_VERIFY(false);
		}

		constexpr bool c_IncludeDebugInfo = true;

		glslang::SpvOptions spvOptions =
		{
			.generateDebugInfo = c_IncludeDebugInfo,
			.stripDebugInfo = !c_IncludeDebugInfo,
			.disableOptimizer = c_IncludeDebugInfo,
			.disassemble = false,
			.validate = true,
			.emitNonSemanticShaderDebugInfo = false,
			.emitNonSemanticShaderDebugSource = false,
		};

		const glslang::TIntermediate* intermediate = program.getIntermediate(lang);

		DynamicArray<uint32_t> result;
		spv::SpvBuildLogger buildLogger;
		glslang::GlslangToSpv(*intermediate, result, &buildLogger, &spvOptions);

		if (!buildLogger.getAllMessages().empty())
		{
			Logging::Warn("SPIR-V messages generated for shader {}!", filepath.string());
			Logging::Warn("Message: {}", buildLogger.getAllMessages());
		}

		return result;
	}

	void VulkanShaderCompiler::CompileModules(VkDevice device, const std::filesystem::path& filepath)
	{
		std::string source;
		YUKI_VERIFY(FileIO::ReadText(filepath, source));

		auto stageSources = PreProcessSource(source);

		auto& shaderData = m_CompiledFiles[filepath];
		shaderData.Name = filepath.stem().string();

		for (const auto&[stage, moduleSource] : stageSources)
		{
			auto code = CompileStage(filepath, stage, moduleSource);

			VkShaderModuleCreateInfo StageData =
			{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = code.size() * sizeof(uint32_t),
				.pCode = code.data(),
			};

			vkCreateShaderModule(device, &StageData, nullptr, &shaderData.Modules[stage]);
		}
	}

}
