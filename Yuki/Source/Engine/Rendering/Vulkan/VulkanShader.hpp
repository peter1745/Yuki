#pragma once

#include "Core/Core.hpp"
#include "Rendering/RHI.hpp"

#include "VulkanInclude.hpp"

namespace Yuki {

	struct VulkanShader
	{
		std::string Name = "";
		Map<ShaderModuleType, VkShaderModule> Modules;
	};

}
