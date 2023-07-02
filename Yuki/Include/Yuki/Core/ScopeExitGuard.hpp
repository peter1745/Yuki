#pragma once

namespace Yuki {

	template<typename TFunction>
	struct ScopeExitGuard
	{
		TFunction Func;

		ScopeExitGuard(TFunction&& InFunction)
			: Func(std::move(InFunction))
		{}

		~ScopeExitGuard() { Func(); }
	};

}

#define YUKI_CONCAT_INTERNAL(a, b) a##b
#define YUKI_CONCAT(a, b) YUKI_CONCAT_INTERNAL(a, b)
#define YUKI_SCOPE_EXIT_GUARD(...) ::Yuki::ScopeExitGuard YUKI_CONCAT(scopeExitGuard, __COUNTER__) = [__VA_ARGS__]
