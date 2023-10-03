#pragma once

#include "Engine/Containers/Span.hpp"

#include "RenderHandles.hpp"

namespace Yuki::RHI {

	struct ImageBarrier
	{
		Span<ImageRH> Images;
		Span<ImageLayout> Layouts;
	};
}
