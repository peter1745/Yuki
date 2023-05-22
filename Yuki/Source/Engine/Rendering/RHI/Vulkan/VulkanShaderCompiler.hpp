#pragma once

#include "Rendering/ShaderManager.hpp"

#include "Rendering/RHI/ShaderCompiler.hpp"

#include "Vulkan.hpp"

namespace Yuki {

	class VulkanShaderCompiler : public ShaderCompiler
	{
	public:
		VulkanShaderCompiler(ShaderManager* InShaderManager, VkDevice InDevice);
		
		ResourceHandle<Shader> CompileFromFile(const std::filesystem::path& InFilePath) override;

	private:
		ShaderManager* m_ShaderManager = nullptr;
		VkDevice m_Device = VK_NULL_HANDLE;
	};

}
