#pragma once

#include "Engine/Common/Core.hpp"

namespace Yuki::RHI {

	enum class RendererFeature
	{
		Core,
		RayTracing
	};
}

YUKI_ENUM_HASH(Yuki::RHI::RendererFeature);