#pragma once

#include "Engine/Common/Core.hpp"
#include "Engine/Containers/Span.hpp"

#include "RenderHandles.hpp"

#include <filesystem>

namespace Yuki::RHI {

	enum class PipelineType
	{
		Rasterization, Raytracing
	};

	struct PipelineShaderInfo
	{
		std::filesystem::path FilePath;
		ShaderStage Stage;
	};

	struct PipelineInfo
	{
		PipelineType Type;
		Span<PipelineShaderInfo> Shaders;
		uint32_t PushConstantSize = 0;
		Span<DescriptorSetLayoutRH> DescriptorLayouts;
	};

}

YUKI_ENUM_HASH(Yuki::RHI::ShaderStage);