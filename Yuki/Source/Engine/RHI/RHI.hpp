#pragma once

#include "Engine/Core/Handle.hpp"

namespace Yuki {

	struct RHIContext : Handle<RHIContext>
	{
		static RHIContext Create();
		void Destroy();
	};

}
