#pragma once

#include "Unique.hpp"
#include "Types.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Yuki {

	template<typename T>
	inline T Cast(auto InValue) { return static_cast<T>(InValue); }

	template<typename T>
	inline T AlignUp(T InValue, T InAlignment)
	{
		return (InValue + InAlignment - 1) & ~(InAlignment - 1);
	}

	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;
	using Quat = glm::quat;
	using Mat4 = glm::mat4;
	
	inline Mat4 PerspectiveInfReversedZ(float InFovY, float InAspect, float InNearZ)
	{
		float F = 1.0f / std::tanf(InFovY / 2.0f);

		Mat4 Result{};
		Result[0][0] = F / InAspect;
		Result[1][1] = F;
		Result[3][2] = InNearZ;
		Result[2][3] = -1.0f;

		return Result;
	}

}

#define YUKI_UNUSED(Var) (void)Var

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

#define YUKI_FLAG_ENUM(Name)																		\
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

#define YUKI_ENUM_HASH(Enum)													\
	template<>																	\
	struct std::hash<Enum>															\
	{																			\
		size_t operator()(Enum InValue) const									\
		{																		\
			auto Hash = std::hash<std::underlying_type_t<decltype(InValue)>>();	\
			return Hash(std::to_underlying(InValue));							\
		}																		\
	}
