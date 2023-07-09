#include "Asset/AssetID.hpp"

#include <random>

namespace Yuki {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_RandomEngine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	AssetID::AssetID(AssetType InType)
	{
		uint64_t id = s_UniformDistribution(s_RandomEngine);

		m_Value = static_cast<uint64_t>(static_cast<uint16_t>(InType)) << TypeMask;
		m_Value |= (id & IDMask);
	}

	AssetID::AssetID(uint64_t InValue)
		: m_Value(InValue)
	{
	}

}
