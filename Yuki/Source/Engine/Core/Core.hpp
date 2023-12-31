#pragma once

namespace Yuki {

	template<typename From, typename To>
	concept CastableTo = requires { static_cast<To>(std::declval<From>()); };

}

#define YukiUnused(x) (void)x
