#pragma once

#include "Unique.hpp"
#include "Types.hpp"

namespace Yuki {

	template<typename T>
	inline T Cast(auto InValue) { return static_cast<T>(InValue); }

}

#define YUKI_SINGLETON(Type)							\
public:													\
	static const Type& Get() { return GetInternal(); }	\
private:												\
	static Type& GetInternal()							\
	{													\
		static Type s_##Type;							\
		return s_##Type;								\
	}													\
	Type() = default;									\
	Type(const Type&) = delete;							\
	Type(Type&&) noexcept = delete;						\
	Type& operator=(const Type&) = delete;				\
	Type& operator=(Type&&) noexcept = delete			\

#define YUKI_MUTABLE_SINGLETON(Type)					\
public:													\
	static Type& Get()									\
	{													\
		static Type s_##Type;							\
		return s_##Type;								\
	}													\
private:												\
	Type() = default;									\
	Type(const Type&) = delete;							\
	Type(Type&&) noexcept = delete;						\
	Type& operator=(const Type&) = delete;				\
	Type& operator=(Type&&) noexcept = delete			\

#define YUKI_ENUM(Name)																		\
	enum class Name;																		\
	constexpr Name operator|(const Name InLHS, const Name InRHS) noexcept					\
	{																						\
		return static_cast<Name>(std::to_underlying(InLHS) | std::to_underlying(InRHS));	\
	}																						\
	constexpr bool operator&(const Name InLHS, const Name InRHS) noexcept					\
	{																						\
		return (std::to_underlying(InLHS) & std::to_underlying(InRHS)) != 0;				\
	}																						\
	constexpr Name operator~(const Name InValue) noexcept									\
	{																						\
		return static_cast<Name>(~std::to_underlying(InValue));								\
	}																						\
	constexpr Name& operator|=(Name& InLHS, const Name& InRHS) noexcept						\
	{																						\
		return (InLHS = (InLHS | InRHS));													\
	}																						\
	enum class Name