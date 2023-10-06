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
	};

	static const HashMap<ShaderStage, EShLanguage> c_EShLanguageLookup =
	{
		{ ShaderStage::Vertex,			EShLangVertex },
		{ ShaderStage::Fragment,		EShLangFragment },
		{ ShaderStage::RayGeneration,	EShLangRayGen },
		{ ShaderStage::RayMiss,			EShLangMiss },
		{ ShaderStage::RayClosestHit,	EShLangClosestHit },
	};

	VulkanShaderCompiler::VulkanShaderCompiler()
	{
		glslang::InitializeProcess();
	}

	VulkanShaderCompiler::~VulkanShaderCompiler()
	{
		glslang::FinalizeProcess();
	}

	VkShaderModule VulkanShaderCompiler::CompileOrGetModule(VkDevice InDevice, const std::filesystem::path& InFilePath, ShaderStage InStage)
	{
		if (!m_CompiledFiles.contains(InFilePath))
			CompileModules(InDevice, InFilePath);

		return m_CompiledFiles.at(InFilePath).Modules.at(InStage);
	}

	HashMap<ShaderStage, std::string> VulkanShaderCompiler::PreProcessSource(std::string_view InSource) const
	{
		size_t StageStart = InSource.find("#stage");
		YUKI_VERIFY(StageStart != std::string_view::npos);

		HashMap<ShaderStage, std::string> Result;

		while (StageStart != std::string_view::npos)
		{
			size_t StageNameStart = StageStart + strlen("#stage");
			size_t StageNameEnd = InSource.find_first_of("\n", StageNameStart);
			std::string_view StageName = InSource.substr(StageNameStart, StageNameEnd - StageNameStart);
			StageName = StringHelper::TrimWhitespace(StageName);

			YUKI_VERIFY(c_StageLookup.contains(StageName));

			ShaderStage Stage = c_StageLookup.at(StageName);

			size_t StageEnd = InSource.find("#stage", StageNameEnd + 1);
			if (StageEnd == std::string_view::npos)
				StageEnd = InSource.length();

			Result[Stage] = InSource.substr(StageNameEnd + 1, StageEnd - StageNameEnd - 1);

			StageStart = InSource.find("#stage", StageNameEnd + 1);
		}

		return Result;
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

	DynamicArray<uint32_t> VulkanShaderCompiler::CompileStage(const std::filesystem::path& InFilePath, ShaderStage InStage, std::string_view InSource) const
	{
		YUKI_VERIFY(c_EShLanguageLookup.contains(InStage));

		auto Lang = c_EShLanguageLookup.at(InStage);

		glslang::TShader Shader{Lang};
		Shader.setEnvInput(glslang::EShSourceGlsl, Lang, glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
		Shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
		Shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_6);

		const char* Source = InSource.data();
		int32_t SourceLength = Cast<int32_t>(InSource.length());
		auto FilePath = InFilePath.string();
		const char* FilePathStr = FilePath.c_str();
		Shader.setStringsWithLengthsAndNames(&Source, &SourceLength, &FilePathStr, 1);

		GlslIncluder Includer;

		const auto* Resource = GetDefaultResources();

		std::string PreProcessed;
		if (!Shader.preprocess(Resource, 450, EEsProfile, false, false, EShMessages::EShMsgEnhanced, &PreProcessed, Includer))
		{
			Logging::Error("Failed to preprocess shader: {}!", InFilePath.string());
			Logging::Error("Reason: {}\n{}", Shader.getInfoLog(), Shader.getInfoDebugLog());
			YUKI_VERIFY(false);
		}

		const char* PreProcessedStr = PreProcessed.c_str();
		Shader.setStrings(&PreProcessedStr, 1);

		if (!Shader.parse(Resource, 450, false, EShMessages::EShMsgDefault))
		{
			Logging::Error("Failed to parse shader: {}!", InFilePath.string());
			Logging::Error("Reason: {}\n{}", Shader.getInfoLog(), Shader.getInfoDebugLog());
			YUKI_VERIFY(false);
		}

		glslang::TProgram Program;
		Program.addShader(&Shader);

		if (!Program.link(EShMessages(int(EShMessages::EShMsgSpvRules) | int(EShMessages::EShMsgVulkanRules))))
		{
			Logging::Error("Failed to link shader: {}!", InFilePath.string());
			Logging::Error("Reason: {}\n{}", Shader.getInfoLog(), Shader.getInfoDebugLog());
			YUKI_VERIFY(false);
		}

		constexpr bool c_IncludeDebugInfo = true;

		glslang::SpvOptions SpvOptions =
		{
			.generateDebugInfo = c_IncludeDebugInfo,
			.stripDebugInfo = !c_IncludeDebugInfo,
			.disableOptimizer = c_IncludeDebugInfo,
			.disassemble = false,
			.validate = true,
			.emitNonSemanticShaderDebugInfo = false,
			.emitNonSemanticShaderDebugSource = false,
		};

		const glslang::TIntermediate* Intermediate = Program.getIntermediate(Lang);

		DynamicArray<uint32_t> Result;
		spv::SpvBuildLogger BuildLogger;
		glslang::GlslangToSpv(*Intermediate, Result, &BuildLogger, &SpvOptions);

		if (!BuildLogger.getAllMessages().empty())
		{
			Logging::Warn("SPIR-V messages generated for shader {}!", InFilePath.string());
			Logging::Warn("Message: {}", BuildLogger.getAllMessages());
		}

		return Result;
	}

	void VulkanShaderCompiler::CompileModules(VkDevice InDevice, const std::filesystem::path& InFilePath)
	{
		std::string Source;
		YUKI_VERIFY(FileIO::ReadText(InFilePath, Source));

		auto StageSources = PreProcessSource(Source);

		auto& ShaderData = m_CompiledFiles[InFilePath];
		ShaderData.Name = InFilePath.stem().string();

		for (const auto&[Stage, ModuleSource] : StageSources)
		{
			auto Code = CompileStage(InFilePath, Stage, ModuleSource);

			VkShaderModuleCreateInfo StageData =
			{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = Code.size() * sizeof(uint32_t),
				.pCode = Code.data(),
			};

			vkCreateShaderModule(InDevice, &StageData, nullptr, &ShaderData.Modules[Stage]);
		}
	}

}
