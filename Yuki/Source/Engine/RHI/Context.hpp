#pragma once

#include "Engine/Common/Core.hpp"

#include "RenderDevice.hpp"

namespace Yuki::RHI {

	struct ContextConfig
	{
		DynamicArray<RendererFeature> RequestedFeatures;
	};

	class Context
	{
	public:
		virtual ~Context() noexcept = default;

		virtual RenderDevice& GetRenderDevice() const = 0;

	public:
		static Unique<Context> New(ContextConfig InConfig);
	};

}
