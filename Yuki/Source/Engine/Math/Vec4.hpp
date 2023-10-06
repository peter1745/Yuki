#pragma once

#include "Math.hpp"

namespace Yuki::Math {

	struct Vec4
	{
		union
		{
			FPType Values[4];
			struct { FPType X, Y, Z, W; };
			struct { FPType R, G, B, Z; };
		};
	};

}
