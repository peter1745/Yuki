#include "UniqueID.hpp"

namespace Yuki {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_RandomEngine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	UniqueID::UniqueID()
		: m_Value(s_UniformDistribution(s_RandomEngine))
	{

	}

	UniqueID::UniqueID(uint64_t InValue)
		: m_Value(InValue)
	{

	}

}
