#pragma once

#include <functional>

namespace Yuki::RHI {

	enum class RendererFeature
	{
		RayTracing
	};
}

namespace std {

	template<>
	struct hash<Yuki::RHI::RendererFeature>
	{
		size_t operator()(Yuki::RHI::RendererFeature InFeature) const
		{
			return std::hash<std::underlying_type_t<decltype(InFeature)>>()(std::to_underlying(InFeature));
		}
	};

}
