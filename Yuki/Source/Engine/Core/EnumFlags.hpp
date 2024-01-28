#pragma once

namespace Yuki {

	template<typename T>
	constexpr inline std::false_type MakeEnumFlags(T) { return {}; }

	template<typename T>
	concept EnumFlags = std::is_scoped_enum_v<T> && !std::same_as<std::false_type, decltype(MakeEnumFlags(std::declval<T>()))>;
}

template<Yuki::EnumFlags TEnum>
constexpr TEnum operator|(TEnum InLHS, TEnum InRHS) noexcept
{
	return static_cast<TEnum>(std::to_underlying(InLHS) | std::to_underlying(InRHS));
}

template <Yuki::EnumFlags TEnum>
constexpr bool operator&(TEnum InLHS, TEnum InRHS) noexcept
{
	return (std::to_underlying(InLHS) & std::to_underlying(InRHS)) != 0;
}

template <Yuki::EnumFlags TEnum>
constexpr TEnum operator^(TEnum InLHS, TEnum InRHS) noexcept
{
	return static_cast<TEnum>(std::to_underlying(InLHS) ^ std::to_underlying(InRHS));
}

template <Yuki::EnumFlags TEnum>
constexpr TEnum operator~(TEnum InValue) noexcept
{
	return static_cast<TEnum>(~std::to_underlying(InValue));
}

template <Yuki::EnumFlags TEnum>
constexpr bool operator!(TEnum InValue) noexcept
{
	return !std::to_underlying(InValue);
}

template <Yuki::EnumFlags TEnum>
constexpr TEnum& operator|=(TEnum& InLHS, const TEnum& InRHS) noexcept
{
	return (InLHS = (InLHS | InRHS));
}

template <Yuki::EnumFlags TEnum>
constexpr TEnum& operator&=(TEnum& InLHS, const TEnum& InRHS) noexcept
{
	return (InLHS = (InLHS & InRHS));
}

template <Yuki::EnumFlags TEnum>
constexpr TEnum& operator^=(TEnum& InLHS, const TEnum& InRHS) noexcept
{
	return (InLHS = (InLHS ^ InRHS));
}

