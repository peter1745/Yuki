#include "ShaderCompiler.hpp"
#include "VulkanRHI.hpp"

#include <Engine/IO/FileIO.hpp>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

namespace Yuki {

	static EShLanguage ShaderStageToEShLanguage(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex: return EShLangVertex;
		case ShaderStage::Fragment: return EShLangFragment;
		}

		YukiAssert(false);
		return EShLangCount;
	}

	ShaderCompiler::ShaderCompiler()
	{
		glslang::InitializeProcess();
	}

	ShaderCompiler::~ShaderCompiler()
	{
		glslang::FinalizeProcess();
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
			delete static_cast<UserData*>(includeResult->userData);
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

			YukiAssert(exists);

			auto userData = new UserData();
			userData->Name = target.string();
			FileIO::ReadText(target, userData->Content);

			return new IncludeResult(userData->Name, userData->Content.c_str(), userData->Content.length(), userData);
		}
	};

	VkShaderModule ShaderCompiler::CompileShader(RHIContext context, const std::filesystem::path& filepath, ShaderStage stage)
	{
		std::string source;
		auto filepathStr = filepath.string();

		if (!FileIO::ReadText(filepath, source))
		{
			WriteLine("Failed to load shader {}, file doesn't exist.", LogLevel::Error, filepathStr);
			return nullptr;
		}

		auto lang = ShaderStageToEShLanguage(stage);
		glslang::TShader shader{ lang };
		shader.setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
		shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
		shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_6);

		const char* sourceStr = source.c_str();
		int32_t sourceLength = static_cast<int32_t>(source.length());
		const char* filepathRawStr = filepathStr.c_str();
		shader.setStringsWithLengthsAndNames(&sourceStr, &sourceLength, &filepathRawStr, 1);

		GlslIncluder includer;

		const auto* defaultResources = GetDefaultResources();

		std::string preProcessed;
		if (!shader.preprocess(defaultResources, 450, ECoreProfile, false, false, EShMsgEnhanced, &preProcessed, includer))
		{
			WriteLine("Failed to pre-process shader {}.", LogLevel::Error, filepathStr);
			WriteLine("Reason: {}", LogLevel::Error, shader.getInfoLog(), shader.getInfoDebugLog());
			return nullptr;
		}

		const char* preProcessedStr = preProcessed.c_str();
		shader.setStrings(&preProcessedStr, 1);

		if (!shader.parse(defaultResources, 450, false, EShMsgEnhanced, includer))
		{
			WriteLine("Failed to parse shader {}.", LogLevel::Error, filepathStr);
			WriteLine("Reason: {}", LogLevel::Error, shader.getInfoLog(), shader.getInfoDebugLog());
			return nullptr;
		}

		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules)))
		{
			WriteLine("Failed to link shader {}.", LogLevel::Error, filepathStr);
			WriteLine("Reason: {}", LogLevel::Error, program.getInfoLog(), program.getInfoDebugLog());
			return nullptr;
		}

		static constexpr bool DebugBuild = true;

		glslang::SpvOptions options =
		{
			.generateDebugInfo = DebugBuild,
			.stripDebugInfo = !DebugBuild,
			.disableOptimizer = DebugBuild,
			.disassemble = false,
			.validate = DebugBuild,
			.emitNonSemanticShaderDebugInfo = false,
			.emitNonSemanticShaderDebugSource = false,
		};

		const auto* intermediate = program.getIntermediate(lang);

		std::vector<uint32_t> shaderBinary;
		spv::SpvBuildLogger buildLogger;
		glslang::GlslangToSpv(*intermediate, shaderBinary, &buildLogger, &options);

		std::string message = buildLogger.getAllMessages();
		if (!message.empty())
		{
			WriteLine("SPIR-V message generated for shader {}!", filepathStr);
			WriteLine("Message: {}", message);
		}

		VkShaderModuleCreateInfo moduleInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = shaderBinary.size() * sizeof(uint32_t),
			.pCode = shaderBinary.data(),
		};

		VkShaderModule shaderModule;
		Vulkan::CheckResult(vkCreateShaderModule(context->Device, &moduleInfo, nullptr, &shaderModule));
		return shaderModule;
	}

}
