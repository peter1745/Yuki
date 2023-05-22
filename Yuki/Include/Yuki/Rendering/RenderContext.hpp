#pragma once

#include "../Memory/Unique.hpp"

#include "RenderAPI.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "ShaderCompiler.hpp"
#include "ShaderManager.hpp"

namespace Yuki {

	class GenericWindow;

	class RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void Initialize() = 0;
		virtual void Destroy() = 0;

		virtual Unique<Swapchain> CreateSwapchain(GenericWindow* InWindow) const = 0;

		virtual ShaderManager* GetShaderManager() const = 0;
		virtual ShaderCompiler* GetShaderCompiler() const = 0;

		virtual Device* GetDevice() const = 0;

		static Unique<RenderContext> New(RenderAPI InRenderAPI);
	};

}
