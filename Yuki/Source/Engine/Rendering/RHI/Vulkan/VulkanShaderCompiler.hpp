#pragma once

#include "Rendering/ShaderManager.hpp"

#include "Rendering/RHI/ShaderCompiler.hpp"

#include "VulkanInclude.hpp"

namespace Yuki {

	class VulkanRenderContext;

	class VulkanShaderCompiler : public ShaderCompiler
	{
	public:
		VulkanShaderCompiler(VulkanRenderContext* InContext);
		
		Unique<Shader> CompileFromFile(const std::filesystem::path& InFilePath) override;

	private:
		VulkanRenderContext* m_Context = nullptr;
	};

}
