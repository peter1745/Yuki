#pragma once

#include "Engine/Core/Handle.hpp"
#include "Engine/RHI/RHI.hpp"

#include <rtmcpp/Vector.hpp>

namespace Yuki {

	struct GeometryBatch : Handle<GeometryBatch>
	{
		void Clear() const;
		void AddQuad(rtmcpp::Vec2 position, rtmcpp::Vec4 color) const;
		void AddTexturedQuad(rtmcpp::Vec2 position, Image image) const;
		void MarkDirty() const;
	};

}
