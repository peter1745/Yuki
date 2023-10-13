#pragma once

#include "Unique.hpp"
#include "Types.hpp"

#pragma warning(push)
#pragma warning(disable: 4201)

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/compatibility.hpp>

#pragma warning(pop)

namespace Yuki {

	template<typename T>
	inline T Cast(auto value) { return static_cast<T>(value); }

	template<typename T>
	inline T AlignUp(T value, T alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}

	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;
	using Quat = glm::quat;
	using Mat4 = glm::mat4;
	
	inline Mat4 PerspectiveInfReversedZ(float fovy, float aspect, float nearZ)
	{
		float f = 1.0f / std::tanf(fovy / 2.0f);

		Mat4 result{};
		result[0][0] = f / aspect;
		result[1][1] = f;
		result[3][2] = nearZ;
		result[2][3] = -1.0f;

		return result;
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
	struct std::hash<Enum>														\
	{																			\
		size_t operator()(Enum value) const										\
		{																		\
			auto hash = std::hash<std::underlying_type_t<decltype(value)>>();	\
			return hash(std::to_underlying(value));								\
		}																		\
	}
