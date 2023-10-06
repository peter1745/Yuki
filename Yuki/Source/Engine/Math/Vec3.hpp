#pragma once

#include "Math.hpp"

namespace Yuki::Math {

	struct Vec3
	{
		union
		{
			FPType Values[3];
			struct { FPType X, Y, Z; };
			struct { FPType R, G, B; };
		};
	};

}
