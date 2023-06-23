#pragma once

#include "Yuki/Core/Debug.hpp"

namespace Yuki {

	enum class Fence{};

	enum class Queue{};

	enum class Swapchain{};

	enum class CommandPool{};
	enum class CommandList{};

	enum class Buffer{};
	enum class BufferType { VertexBuffer, IndexBuffer, StorageBuffer, StagingBuffer };
	struct BufferInfo
	{
		BufferType Type;
		uint32_t Size;
		bool PersitentlyMapped = false;
	};

	enum class Image{};
	enum class ImageView{};
	enum class ImageLayout { Attachment, ShaderReadOnly, Present };
	enum class ImageFormat
	{
		None = -1,

		RGBA8UNorm,
		BGRA8UNorm,
		Depth32SFloat
	};
	static constexpr bool IsDepthFormat(ImageFormat InFormat) { return InFormat == ImageFormat::Depth32SFloat; }

	enum class Shader{};
	enum class ShaderModuleType
	{
		None = -1,
		Vertex,
		Fragment
	};
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

	enum class Pipeline{};

}
