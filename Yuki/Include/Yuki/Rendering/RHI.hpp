#pragma once

#include "Yuki/Core/Core.hpp"
#include "Yuki/Core/Debug.hpp"
#include "Yuki/Core/EnumFlags.hpp"

namespace Yuki {

	enum class FenceHandle{};

	enum class QueueHandle{};

	enum class SwapchainHandle{};

	enum class CommandPoolHandle{};
	enum class CommandListHandle{};

	enum class BufferHandle{};
	enum class BufferType { VertexBuffer, IndexBuffer, StorageBuffer, StagingBuffer };
	struct BufferInfo
	{
		BufferType Type;
		uint32_t Size;
	};

	enum class ImageHandle{};
	enum class ImageViewHandle{};
	enum class ImageLayout { Undefined = -1, Attachment, ShaderReadOnly, Present, TransferDestination, TransferSource };
	enum class ImageFormat
	{
		None = -1,

		RGBA8UNorm,
		BGRA8UNorm,
		Depth32SFloat
	};
	static constexpr bool IsDepthFormat(ImageFormat InFormat) { return InFormat == ImageFormat::Depth32SFloat; }
	enum class ImageUsage
	{
		ColorAttachment = 1 << 0,
		DepthAttachment = 1 << 1,
		Sampled = 1 << 2,
		TransferDestination = 1 << 3,
		TransferSource = 1 << 4
	};
	YUKI_ENUM_FLAGS(ImageUsage);

	enum class SamplerHandle{};

	enum class ShaderHandle{};
	enum class ShaderModuleType
	{
		None = -1,
		Vertex,
		Fragment
	};
	enum class ShaderStage
	{
		None = -1,
		Vertex = 1 << 0,
		Fragment = 1 << 1,
	};
	YUKI_ENUM_FLAGS(ShaderStage);
	enum class ShaderDataType
	{
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		UInt, UInt2, UInt3, UInt4,
	};
	static constexpr uint32_t ShaderDataTypeSize(ShaderDataType InType)
	{
		switch (InType)
		{
		case ShaderDataType::Float: return sizeof(float);
		case ShaderDataType::Float2: return sizeof(float) * 2;
		case ShaderDataType::Float3: return sizeof(float) * 3;
		case ShaderDataType::Float4: return sizeof(float) * 4;
		case ShaderDataType::Int: return sizeof(int32_t);
		case ShaderDataType::Int2: return sizeof(int32_t) * 2;
		case ShaderDataType::Int3: return sizeof(int32_t) * 3;
		case ShaderDataType::Int4: return sizeof(int32_t) * 4;
		case ShaderDataType::UInt: return sizeof(uint32_t);
		case ShaderDataType::UInt2: return sizeof(uint32_t) * 2;
		case ShaderDataType::UInt3: return sizeof(uint32_t) * 3;
		case ShaderDataType::UInt4: return sizeof(uint32_t) * 4;
		}

		YUKI_VERIFY(false);
		return 0;
	}

	enum class PipelineHandle{};
	enum class PolygonModeType { Fill, Line };

	enum class DescriptorPoolHandle{};
	enum class DescriptorSetHandle{};
	enum class DescriptorSetLayoutHandle{};
	enum class DescriptorType { CombinedImageSampler, StorageBuffer };
	struct DescriptorSetLayoutInfo
	{
		ShaderStage Stages = ShaderStage::None;
		struct DescriptorInfo { uint32_t Count; DescriptorType Type; };
		DynamicArray<DescriptorInfo> Descriptors;
	};
	struct DescriptorCount
	{
		DescriptorType Type;
		uint32_t Count;
	};

	struct Scissor
	{
		float X, Y;
		float Width, Height;
	};


}
