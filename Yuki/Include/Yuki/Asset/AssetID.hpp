#pragma once

#include "AssetTypes.hpp"

namespace Yuki {

	/// <summary>
	/// An opaque handle to an asset (loaded or unloaded)
	/// Represented as a single 64-bit unsigned integer, with the asset type
	/// stored in the 8 most signifigant bits
	/// </summary>
	class AssetID
	{
	private:
		static constexpr uint64_t TypeMask = 64 - 8;
		static constexpr uint64_t IDMask = 0xFF'FFFF'FFFF'FFFFULL;

	public:
		AssetID() = default;
		AssetID(uint64_t InValue);
		AssetID(AssetType InType);

		operator uint64_t() { return m_Value; }
		operator const uint64_t() const { return m_Value; }

		AssetType GetType() const { return static_cast<AssetType>(m_Value >> TypeMask); }

		bool IsValid() const { return m_Value != 0; }

	private:
		uint64_t m_Value = 0;
	};

	struct AssetIDHash
	{
		using is_avalanching = void;

		uint64_t operator()(const AssetID& InID) const noexcept
		{
			return ankerl::unordered_dense::detail::wyhash::hash(&InID, sizeof(AssetID));
		}
	};

}
