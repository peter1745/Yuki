#pragma once

#define YUKI_ENUM_FLAGS(Enum)\
	constexpr Enum operator|(const Enum InLHS, const Enum InRHS) noexcept\
	{\
		return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(InLHS) | static_cast<std::underlying_type_t<Enum>>(InRHS));\
	}\
	constexpr bool operator&(const Enum InLHS, const Enum InRHS) noexcept\
	{\
		return (static_cast<std::underlying_type_t<Enum>>(InLHS) & static_cast<std::underlying_type_t<Enum>>(InRHS)) != 0;\
	}\
	constexpr Enum operator^(const Enum InLHS, const Enum InRHS) noexcept\
	{\
		return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(InLHS) ^ static_cast<std::underlying_type_t<Enum>>(InRHS));\
	}\
	constexpr Enum operator~(const Enum InValue) noexcept\
	{\
		return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(InValue));\
	}\
	constexpr bool operator!(const Enum InValue) noexcept\
	{\
		return !static_cast<std::underlying_type_t<Enum>>(InValue);\
	}\
	constexpr Enum& operator|=(Enum& InLHS, const Enum& InRHS) noexcept\
	{\
		return (InLHS = (InLHS | InRHS));\
	}\
	constexpr Enum& operator^=(Enum& InLHS, const Enum& InRHS) noexcept\
	{\
		return (InLHS = (InLHS ^ InRHS));\
	}

#if 0

namespace Yuki {

	struct EnumFlagsBase {};

	template<typename TEnum>
	struct EnumFlags {};

	template<typename TEnum>
	inline constexpr bool IsScopedEnumV = std::conjunction_v<std::is_enum<TEnum>, std::negation<std::is_convertible<TEnum, int>>>;

	template<typename TEnum>
	concept IsEnumFlags =
		std::is_default_constructible_v<EnumFlags<TEnum>> &&
		std::is_constructible_v<EnumFlags<TEnum>> &&
		(IsScopedEnumV<TEnum> || std::is_enum_v<TEnum>) &&
		std::is_base_of_v<EnumFlagsBase, EnumFlags<TEnum>>;

}

template<typename TEnum>
requires Yuki::IsEnumFlags<TEnum>
constexpr TEnum operator|(TEnum InLHS, TEnum InRHS) noexcept
{
	return static_cast<TEnum>(static_cast<std::underlying_type_t<TEnum>>(InLHS) | static_cast<std::underlying_type_t<TEnum>>(InRHS));
}

template <typename TEnum>
requires Yuki::IsEnumFlags<TEnum>
constexpr TEnum operator&(TEnum InLHS, TEnum InRHS) noexcept
{
	return static_cast<TEnum>(static_cast<std::underlying_type_t<TEnum>>(InLHS) & static_cast<std::underlying_type_t<TEnum>>(InRHS));
}

template <typename TEnum>
requires Yuki::IsEnumFlags<TEnum>
constexpr bool operator!=(TEnum InLHS, std::underlying_type_t<TEnum> InRHS) noexcept
{
	return static_cast<std::underlying_type_t<TEnum>>(InLHS) != InRHS;
}

template<typename TEnum>
requires Yuki::IsEnumFlags<TEnum>
constexpr bool operator==(TEnum InLHS, std::underlying_type_t<TEnum> InRHS) noexcept
{
	return static_cast<std::underlying_type_t<TEnum>>(InLHS) == InRHS;
}

template <typename TEnum>
requires Yuki::IsEnumFlags<TEnum>
constexpr TEnum operator^(TEnum InLHS, TEnum InRHS) noexcept
{
	return static_cast<TEnum>(static_cast<std::underlying_type_t<TEnum>>(InLHS) ^ static_cast<std::underlying_type_t<TEnum>>(InRHS));
}

template <typename TEnum>
requires Yuki::IsEnumFlags<TEnum>
constexpr TEnum operator~(TEnum InValue) noexcept
{
	return static_cast<TEnum>(~static_cast<std::underlying_type_t<TEnum>>(InValue));
}

template <typename TEnum>
requires Yuki::IsEnumFlags<TEnum>
constexpr bool operator!(TEnum InValue) noexcept
{
	return !static_cast<std::underlying_type_t<TEnum>>(InValue);
}

template <typename TEnum>
requires Yuki::IsEnumFlags<TEnum>
constexpr TEnum& operator|=(TEnum& InLHS, const TEnum& InRHS) noexcept
{
	return (InLHS = (InLHS | InRHS));
}

template <typename TEnum>
requires Yuki::IsEnumFlags<TEnum>
constexpr TEnum& operator&=(TEnum& InLHS, const TEnum& InRHS) noexcept
{
	return (InLHS = (InLHS & InRHS));
}

template <typename TEnum>
requires Yuki::IsEnumFlags<TEnum>
constexpr TEnum& operator^=(TEnum& InLHS, const TEnum& InRHS) noexcept
{
	return (InLHS = (InLHS ^ InRHS));
}

#endif