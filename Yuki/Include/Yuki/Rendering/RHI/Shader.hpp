#pragma once

#include "Yuki/Core/ResourceHandle.hpp"

// I fucking hate Xlib (They define None)
#ifdef None
	#undef None
#endif

namespace Yuki {

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

		return 0;
	}

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual std::string_view GetName() const = 0;
	};

}
