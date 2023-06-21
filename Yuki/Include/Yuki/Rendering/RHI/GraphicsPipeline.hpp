#pragma once

#include "Yuki/Core/ResourceHandle.hpp"

namespace Yuki {

	class Shader;

	class GraphicsPipeline
	{
	public:
		virtual ~GraphicsPipeline() = default;

		virtual Shader* GetShader() = 0;
	};

}
