#pragma once

#include "Yuki/Core/EnumFlags.hpp"

#include "DescriptorSet.hpp"

namespace Yuki {

	enum class ShaderStage
	{
		None = -1,
		Vertex = 1 << 0,
		Fragment = 1 << 1
	};
	YUKI_ENUM_FLAGS(ShaderStage);

	class SetLayoutBuilder
	{
	public:
		virtual ~SetLayoutBuilder() = default;

		virtual SetLayoutBuilder& Start() = 0;
		virtual SetLayoutBuilder& Stages(ShaderStage InStages) = 0;
		virtual SetLayoutBuilder& Binding(uint32_t InDescriptorCount, DescriptorType InType) = 0;
		virtual DescriptorSetLayout* Build() = 0;

	};

}
