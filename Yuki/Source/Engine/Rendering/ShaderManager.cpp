#include "Rendering/ShaderManager.hpp"

namespace Yuki {

	ResourceHandle<Shader> ShaderManager::AddShader(Shader* InShader)
	{
		ResourceHandle<Shader> handle;
		m_Shaders[handle] = Unique<Shader>(InShader);
		return handle;
	}

	Shader* ShaderManager::GetShader(ResourceHandle<Shader> InHandle) const
	{
		YUKI_VERIFY(InHandle.IsValid());
		YUKI_VERIFY(m_Shaders.contains(InHandle));
		return m_Shaders.at(InHandle).GetPtr();
	}

}
