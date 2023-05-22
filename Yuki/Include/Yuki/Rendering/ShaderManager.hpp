#pragma once

#include "RHI/Shader.hpp"

namespace Yuki {

	class ShaderManager
	{
	public:
		ResourceHandle<Shader> AddShader(Shader* InShader);
		Shader* GetShader(ResourceHandle<Shader> InHandle) const;

	private:
		Map<ResourceHandle<Shader>, Unique<Shader>, ResourceHandleHash<Shader>> m_Shaders;
	};

}
