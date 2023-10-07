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
		Span<PipelineShaderInfo> Shaders;
		uint32_t PushConstantSize = 0;
		Span<DescriptorSetLayoutRH> DescriptorLayouts;

		struct ColorAttachmentInfo { ImageFormat Format; };
		DynamicArray<ColorAttachmentInfo> ColorAttachments;
	};

	struct RayTracingPipelineInfo
	{
		Span<PipelineShaderInfo> Shaders;
		uint32_t PushConstantSize = 0;
		Span<DescriptorSetLayoutRH> DescriptorLayouts;
	};

}

YUKI_ENUM_HASH(Yuki::RHI::ShaderStage);