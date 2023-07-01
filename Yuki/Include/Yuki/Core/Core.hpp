#pragma once

#include <array>
#include <vector>
#include <ankerl/unordered_dense.h>

namespace Yuki {

	template<typename T, size_t Size>
	using Array = std::array<T, Size>;

	template<typename T>
	using DynamicArray = std::vector<T>;

	template<typename TKey, typename TValue, typename THash = ankerl::unordered_dense::v4_0_0::hash<TKey>>
	using Map = ankerl::unordered_dense::map<TKey, TValue, THash>;

	enum class Platform { Windows, Linux };

#if defined(YUKI_PLATFORM_WINDOWS)
	static constexpr Platform s_Platform = Platform::Windows;
#elif defined(YUKI_PLATFORM_LINUX)
	static constexpr Platform s_Platform = Platform::Linux;
#else
	#error Unsupported Platform!
#endif

	enum class Configuration { Debug, RelWithDebug, Release };
	
#if defined(YUKI_CONFIG_DEBUG)
	static constexpr Configuration s_CurrentConfig = Configuration::Debug;
#elif defined(YUKI_CONFIG_REL_WITH_DEBUG)
	static constexpr Configuration s_CurrentConfig = Configuration::RelWithDebug;
#elif defined(YUKI_CONFIG_RELEASE)
	static constexpr Configuration s_CurrentConfig = Configuration::Release;
#else
	#error Unknown Configuration!
#endif

	#define YUKI_UNUSED(x) (void)x

}
